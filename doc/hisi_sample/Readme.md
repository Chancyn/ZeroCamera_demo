# readme
修改海思sample,如何将venc编码数据写入共享内存fifo
1.打开fifo,zc_shmstreamw_create
2.每一帧帧数据来之后，封装zc_frame_t帧数据头，先zc_shmstreamw_put写入帧数据头
3.再通过回调函数追加写入H264帧数据
···
4.结束后,不需要写入fifo,关闭fifo,zc_shmfifow_destroy
## sample_comm_venc.c文件修改示例

### 1.打开fifo，并注册写入回调函数
```
g_zctestfifow[0] = zc_shmstreamw_create(ZC_RTSP_TEST_MAIN_SIZE, ZC_RTSP_TEST_SHM_PATH, 0, zc_puting_cb_0, &g_zctestfifow[0]);
g_zctestfifow[1] = zc_shmstreamw_create(ZC_RTSP_TEST_SUB_SIZE, ZC_RTSP_TEST_SHM_PATH, 1, zc_puting_cb_1, &g_zctestfifow[1]);
```

### 2.写入回调函数,1帧数据多个nalu,pack;通过回调函数中zc_shmstreamw_put_appending追加数据
```
static unsigned int zc_puting_cb_0(void *u, void *s)
{
    void **fifo = (void *)u;
    hi_venc_stream *stream = (hi_venc_stream *)s;
    hi_u32 i;
    unsigned int ret = 0;
    for (i = 0; i < stream->pack_cnt; i++) {
        ret = zc_shmstreamw_put_appending(g_zctestfifow[0], stream->pack[i].addr + stream->pack[i].offset,
        stream->pack[i].len - stream->pack[i].offset);
    }

    return ret;
}
static unsigned int zc_puting_cb_1(void *u, void *s)
{
    void **fifo = (void *)u;
    hi_venc_stream *stream = (hi_venc_stream *)s;
    hi_u32 i;
    unsigned int ret = 0;
    for (i = 0; i < stream->pack_cnt; i++) {
        ret = zc_shmstreamw_put_appending(g_zctestfifow[1], stream->pack[i].addr + stream->pack[i].offset,
        stream->pack[i].len - stream->pack[i].offset);
    }

    return ret;
}
```

### 3.帧写入函数，封装zc_frame_t帧数据头，先zc_shmstreamw_put写入帧数据头，再通过回调函数写入H264帧数据
```
static hi_s32 zc_sample_comm_streamw_put(hi_s32 index, hi_venc_stream *stream, hi_venc_stream_buf_info *stream_buf_info, hi_payload_type *payload_type)
{
    if (g_zctestfifow[index] == NULL){
        return -1;
    }

    zc_frame_t zcframe;
	struct timespec _ts;
    clock_gettime(CLOCK_MONOTONIC, &_ts);
    zcframe.magic = ZC_RTSP_VIDEO_MAGIC;
    zcframe.type = ZC_STREAM_VIDEO;
    zcframe.keyflag = (stream->pack_cnt > 1)?1:0;
    zcframe.seq  = stream->seq;
    zcframe.utc  = _ts.tv_sec*1000 + _ts.tv_nsec/1000000;
    zcframe.pts  = stream->pack[0].pts/1000;
    if (payload_type[index] == HI_PT_H265) {
        zcframe.video.encode = ZC_FRAME_ENC_H265;
        if (stream->h265_info.ref_type == HI_VENC_BASE_IDR_SLICE) {
            zcframe.keyflag = 1;
            zcframe.video.frame = ZC_FRAME_IDR;
        }
    } else if (payload_type[index] == HI_PT_H264) {
        zcframe.video.encode = ZC_FRAME_ENC_H264;
        if (stream->h264_info.ref_type == HI_VENC_BASE_IDR_SLICE) {
            zcframe.keyflag = 1;
            zcframe.video.frame = ZC_FRAME_IDR;
        }
    }

    zcframe.video.width  = g_send_frame_param.size[index].width;
    zcframe.video.height = g_send_frame_param.size[index].height;
    zcframe.size = 0;
    hi_u32 i = 0;
    for (i = 0; i < stream->pack_cnt; i++) {
        zcframe.size += stream->pack[i].len - stream->pack[i].offset;
    }
    zc_shmstreamw_put(g_zctestfifow[index], &zcframe, sizeof(zcframe), stream);

    // if (zcframe.keyflag) {
    //     LOG_WARN("index[%d] put IDR len[%d], pts[%u],utc[%u], cnt[%d], wh[%u*%u]", index, zcframe.size,
    //     zcframe.pts, zcframe.utc, stream->pack_cnt, zcframe.video.width, zcframe.video.height);
    // }

    return 0;
}
```

### 4.关闭shmfifo
```
zc_shmfifow_destroy(g_zctestfifow[0]);
zc_shmfifow_destroy(g_zctestfifow[1]);
```

