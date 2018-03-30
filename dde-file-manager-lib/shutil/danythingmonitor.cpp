
#include "tag/tagmanager.h"
#include "danythingmonitor.h"
#include "dfileinfo.h"


#ifdef __cplusplus
extern "C"
{
#endif

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#ifdef __cplusplus
}
#endif

#include <QString>



#ifdef __cplusplus
extern "C"
{
#endif


typedef struct __vc_ioctl_readdata_args__ {
    int size;
    char* data;
} ioctl_rd_args;

typedef struct __vc_ioctl_readstat_args__ {
    int total_changes;
    int cur_changes;
    int discarded;
    int cur_memory;
} ioctl_rs_args;

#define VC_IOCTL_MAGIC       0x81
#define VC_IOCTL_READDATA    _IOR(VC_IOCTL_MAGIC, 0, long)
#define VC_IOCTL_READSTAT    _IOR(VC_IOCTL_MAGIC, 1, long)

#ifdef __cplusplus
}
#endif


enum : int
{
    ACT_NEW_FILE =	0,
    ACT_NEW_LINK,
    ACT_NEW_SYMLINK,
    ACT_NEW_FOLDER,
    ACT_DEL_FILE,
    ACT_DEL_FOLDER,
    ACT_RENAME_FILE,
    ACT_RENAME_FOLDER

};

static const char* const PROTOCOL_HEAD{ "file://" };
static const char* const PROCFS_PATH{ "/proc/vfs_changes" };
constexpr static const char* const act_names[]{"file_created", "link_created",
                                               "symlink_created", "dir_created",
                                               "file_deleted", "dir_deleted",
                                               "file_renamed", "dir_renamed"};



static QString StartPoint{};

DAnythingMonitor::DAnythingMonitor()
{
    ///###: constructor.
}


void DAnythingMonitor::doWork()
{

    std::unique_lock<std::mutex> raiiLock{ m_mutex };
    m_conditionVar.wait(raiiLock, [this]{ return m_readyFlag.load(std::memory_order_consume); });
    m_readyFlag.store(false, std::memory_order_release);

    if(!m_changedFiles.empty()){
        std::deque<std::pair<QString, QString>>::const_iterator cbeg{ m_changedFiles.cbegin() };
        std::deque<std::pair<QString, QString>>::const_iterator cend{ m_changedFiles.cend() };

        for(; cbeg != cend; ++cbeg){

            if(cbeg->first.isEmpty()){
                TagManager::instance()->deleteFiles({cbeg->second});
                continue;
            }

            TagManager::instance()->changeTagName({cbeg->first, cbeg->second});
            std::this_thread::sleep_for( std::chrono::duration<std::size_t, std::ratio<1, 1000>>{50} );
        }
    }
}

void DAnythingMonitor::workSignal()
{
    int fd = open(PROCFS_PATH, O_RDONLY);
    if (fd < 0){
        m_readyFlag.store(true, std::memory_order_release);
        m_conditionVar.notify_one();
        return;
    }

    ioctl_rs_args irsa;
    if (ioctl(fd, VC_IOCTL_READSTAT, &irsa) != 0) {
        close(fd);
        m_readyFlag.store(true, std::memory_order_release);
        m_conditionVar.notify_one();
        return;
    }

    if (irsa.cur_changes == 0) {
        close(fd);
        m_readyFlag.store(true, std::memory_order_release);
        m_conditionVar.notify_one();
        return;
    }

    char buf[1<<20]{};
    ioctl_rd_args ira ;
    ira.data = buf;

    {
        std::lock_guard<std::mutex> raiiLock{ m_mutex };
        while(true){
            ira.size = sizeof(buf);
            if (ioctl(fd, VC_IOCTL_READDATA, &ira) != 0)
                break;

            // no more changes
            if (ira.size == 0)
                break;

            int off = 0;
            for (int i = 0; i < ira.size; i++) {
                unsigned char action = *(ira.data + off);
                off++;
                char* src = ira.data + off, *dst = 0;
                off += strlen(src) + 1;

                switch(action)
                {
                ///###: do not delete this.
//                case ACT_NEW_FILE:
//                case ACT_NEW_SYMLINK:
//                case ACT_NEW_LINK:
//                case ACT_NEW_FOLDER:
//                {
//                    qDebug()<< act_names[action] << "-------->" << src;
//                }
                case ACT_DEL_FILE:
                case ACT_DEL_FOLDER:
                {
                    m_changedFiles.emplace_back(QString{}, QString{PROTOCOL_HEAD} + QString{src});
//                    qDebug()<< act_names[action] << "--------->" << src;
                    break;
                }
                case ACT_RENAME_FILE:
                case ACT_RENAME_FOLDER:
                {
                    dst = ira.data + off;
                    off += strlen(dst) + 1;
                    m_changedFiles.emplace_back(QString{PROTOCOL_HEAD} + QString{src},
                                                QString{PROTOCOL_HEAD} + QString{dst});
//                    qDebug()<< act_names[action] << src << "--------->" << dst;
                    break;
                }
                default:
                    break;
                }
            }
        }
        close(fd);
    }

    m_readyFlag.store(true, std::memory_order_release);
    m_conditionVar.notify_one();
}







