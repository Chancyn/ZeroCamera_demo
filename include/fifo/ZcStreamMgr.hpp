// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <list>
#include <mutex>

#include "zc_stream_mgr.h"
#include "zc_type.h"

#include "Singleton.hpp"
#include "Thread.hpp"

namespace zc {
typedef struct _shmname {
    char name[32];                        // stream type name
    char tracksname[ZC_STREAM_BUTT][32];  // shm path
} shmname_t;

typedef struct _mgr_shmname {
    shmname_t tabs[ZC_SHMSTREAM_TYPE_BUTT];
} mgr_shmname_t;

// TODO(zhoucc): MgrCli; -> Mgr save handle
class CStreamMgr : public Thread, public Singleton<CStreamMgr> {
 public:
    CStreamMgr();
    virtual ~CStreamMgr();

 public:
    bool Init(zc_stream_mgr_cfg_t *cfg);
    bool UnInit();
    bool Start();
    bool Stop();


    // ctrl interface
    int HandleCtrl(unsigned int type, void *indata, void *outdata);

 private:
    int getCount(unsigned int type);
    int getALLShmStreamInfo(zc_shmstream_info_t *info, unsigned int type, unsigned int count);
    int getShmStreamInfo(zc_shmstream_info_t *info, unsigned int type, unsigned int nchn);

    bool _unInit();
    virtual int process();
    int _findIdx(zc_shmstream_type_e type, unsigned int nchn);
    inline int _getShmStreamInfo(zc_shmstream_info_t *info, int idx, unsigned int count);
    int _getALLShmStreamInfo(zc_shmstream_info_t *info);
    int _setShmStreamInfo(zc_shmstream_info_t *info, int idx);
    int setShmStreamInfo(zc_shmstream_info_t *info, unsigned int type, unsigned int nchn);
    void _initTracksInfo(zc_shmstream_track_t *info, unsigned char type, unsigned char chn, unsigned char venc,
                         unsigned char aenc, unsigned char menc);

 private:
    bool m_init;
    int m_running;
    unsigned int m_total;
    mgr_shmname_t m_nametab;
    zc_stream_mgr_cfg_t m_cfg;
    zc_shmstream_info_t *m_infoTab;
    std::mutex m_mutex;
    std::list<zc_streamcli_t> m_listcli;
    std::mutex m_listmutex;
};

#define g_ZCStreamMgrInstance (zc::CStreamMgr::GetInstance())
}  // namespace zc