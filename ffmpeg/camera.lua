local ffi, bit = require'ffi', require'bit'
local FFmpeg, ret = loadfile('init.lua')
assert(FFmpeg, ret)
FFmpeg = FFmpeg[[Z:\develop\ffmpeg-3.4.2-win64]]
----------------------------------------------------------------------------------
local _OPT, ret = loadfile('ff-opt.lua')
assert(_OPT, ret)
_OPT = _OPT(FFmpeg, arg)
----------------------------------------------------------------------------------
local _TTY, ret = loadfile('ff-tty.lua')
assert(_TTY, ret)
_TTY = _TTY(FFmpeg, _OPT)
----------------------------------------------------------------------------------
local function open_output(index, i_stm)
    local fmt_ctx = ffi.new('AVFormatContext*[1]')
    local ret = FFmpeg.avformat_alloc_output_context2(fmt_ctx, nil, _OPT.f,  _OPT[index])
    FFmpeg.assert(ret, 'avformat_alloc_output_context2')
    local stream = FFmpeg.avformat_new_stream(fmt_ctx[0], nil)
    stream.time_base.num = 1
    stream.time_base.den = 25
    local codecpar = stream.codecpar

    codecpar.codec_type = i_stm.codecpar.codec_type
    codecpar.format = _OPT.pix_fmt  or i_stm.codecpar.format
    codecpar.codec_id = _OPT.codec.v or i_stm.codecpar.codec_id
    codecpar.width = i_stm.codecpar.width
    codecpar.height = i_stm.codecpar.height
    local size = _OPT.s.v or _OPT.s[index]
    if size then
        local w, h = string.match(size, '^(%d*)x(%d*)$')
        assert(w and h, size)
        codecpar.width, codecpar.height = tonumber(w), tonumber(h)
    end
    --���������AVCodecContext
    local codec = FFmpeg.avcodec_find_encoder(codecpar.codec_id)
    local codec_cxt = FFmpeg.avcodec_alloc_context3(codec);
    local ret = FFmpeg.avcodec_parameters_to_context(codec_cxt, codecpar)
    codec_cxt.time_base = stream.time_base
    FFmpeg.assert(ret, 'avcodec_parameters_to_context')
    local dict = _OPT.codec_opts(codecpar.codec_id, fmt_ctx[0], stream, codec)
    local ret = FFmpeg.avcodec_open2(codec_cxt, codec, dict)
    FFmpeg.assert(ret, 'avcodec_open2')
    FFmpeg.av_dump_format(fmt_ctx[0], 0, _OPT[index], 1);
    return fmt_ctx, stream, codec_cxt
end
local function open_input(index, options)
    local fmt_ctx = ffi.new('AVFormatContext*[1]')
    local ret = FFmpeg.avformat_open_input(fmt_ctx, _OPT.i[index], _OPT.iformat[index], options)
    FFmpeg.assert(ret, 'avformat_open_input')

    local ret = FFmpeg.avformat_find_stream_info(fmt_ctx[0], nil)
    FFmpeg.assert(ret, 'avformat_find_stream_info')
    --���������Ϣ:
    FFmpeg.av_dump_format(fmt_ctx[0], 0, _OPT.i[index], 0);
    --[[print[[
    tbr����֡��
    tbn�����ļ��㣨st����ʱ�侫�ȣ���1S=1200k����duration���
    tbc������Ƶ�㣨st->codec����ʱ�侫�ȣ���1S=XX����stream->duration��ʱ������)
    ]]
    --Ѱ����Ƶ��������˷������ͷͬʱ������Ƶ������
    local videoStreamIndex
    for i=0, fmt_ctx[0].nb_streams-1 do
        if fmt_ctx[0].streams[i].codec.codec_type == FFmpeg.AVMEDIA_TYPE_VIDEO then
            videoStreamIndex = i
            break
        end
    end
    assert(videoStreamIndex)

    local stream = fmt_ctx[0].streams[videoStreamIndex]
    local codecpar = stream.codecpar

    --print ('������Ƶ��ʽ:', ffi.string(FFmpeg.avcodec_get_name(codecpar.codec_id)))
    --print ('������Ƶ�߶�:', codecpar.height)
    --print ('������Ƶ���:', codecpar.width)
    --��ȡ֡����Ϣ��ע��������������r_frame_rate����avg_frame_rate
    --print ('������Ƶ֡��:', stream.r_frame_rate.num * 1.0 /  stream.r_frame_rate.den)

    --���������AVCodecContext
    local codec = FFmpeg.avcodec_find_decoder(codecpar.codec_id)
    local codec_cxt = FFmpeg.avcodec_alloc_context3(codec);
    local ret = FFmpeg.avcodec_parameters_to_context(codec_cxt, codecpar)
    FFmpeg.assert(ret, 'avcodec_parameters_to_context')
    --codec_cxt.pix_fmt = FFmpeg.AV_PIX_FMT_YUV420P
    --filter_codec_opts
    local dict = _OPT.codec_opts(codecpar.codec_id, fmt_ctx[0], stream, codec)
    local ret = FFmpeg.avcodec_open2(codec_cxt, codec, dict)
    FFmpeg.assert(ret, 'avcodec_open2')

    return fmt_ctx, stream, codec_cxt
end
----------------------------------------------------------------------------------
local function process(arg)
    local i_ctx, i_stm, decoder= open_input(1)
    local o_ctx, o_stm, encoder= open_output(1, i_stm)
----------------------------------------------------------------------------------
local i_par, o_par = i_stm.codecpar,o_stm.codecpar
local s_ctx = FFmpeg.sws_getCachedContext(nil,
i_par.width, i_par.height, i_par.format,
o_par.width, o_par.height, o_par.format,
FFmpeg.SWS_BICUBIC, nil, nil, nil)
assert(s_ctx)
--------------------------------------------------------------------------------
--open the output file, if needed
if bit.band(o_ctx[0].oformat.flags, FFmpeg.AVFMT_NOFILE) == 0 then 
    if not _OPT.y and io.open(_OPT[1], 'r') then
        if _OPT.n then
            FFmpeg.av_log(nil, FFmpeg.AV_LOG_ERROR,
            "File '%s' already exists. Exiting.", _OPT[1])
            os.exit(0)
        else
            FFmpeg.av_log(nil, FFmpeg.AV_LOG_ERROR,
            "File '%s' already exists. Overwrite ? [y/N]", _OPT[1])
            if io.read('*l') ~= 'y' then os.exit(0) end
        end
    end
    local pb = ffi.new('AVIOContext*[1]')
    ret = FFmpeg.avio_open(pb, _OPT[1], FFmpeg.AVIO_FLAG_WRITE)
    assert(ret, _OPT[1])
    o_ctx[0].pb = pb[0]
end
--local options = ffi.new('AVDictionary*[1]')
--FFmpeg.av_dict_set(options, 'window_title', 'dshow-sdl', 0);
--FFmpeg.av_dict_set(options, 'window_borderless', '0', 0);
local ret = FFmpeg.avformat_write_header(o_ctx[0], nil)
assert(ret)
----------------------------------------------------------------------------------
local frame = FFmpeg.av_frame_alloc();
local packet = FFmpeg.av_packet_alloc();
local frame2 = FFmpeg.av_frame_alloc();
frame2.format = o_par.format;
frame2.width  = o_par.width;
frame2.height = o_par.height;
FFmpeg.av_frame_get_buffer(frame2, 32);
while not _TTY.sigterm do
    local cur_time= FFmpeg.av_gettime_relative()
    if _TTY.check(cur_time) < 0 then break end

    ret = FFmpeg.av_read_frame(i_ctx[0], packet)
    if ret < 0 then break end
    ret = FFmpeg.avcodec_send_packet(decoder, packet);
    while ret == 0 do
        ret = FFmpeg.avcodec_receive_frame(decoder, frame);
        if ret == 0 then
            FFmpeg.sws_scale(s_ctx, ffi.cast('const unsigned char *const*', frame.data), frame.linesize, 0, i_par.height, frame2.data, frame2.linesize)
            ret = FFmpeg.avcodec_send_frame(encoder, frame2);
        end
        while ret == 0 do
            ret = FFmpeg.avcodec_receive_packet(encoder, packet);
            if ret == 0 then 
                ret = FFmpeg.av_write_frame(o_ctx[0], packet);
                if ret ~= 0 then goto clean end
            end
        end
        FFmpeg.av_frame_unref(frame);
    end
    FFmpeg.av_packet_unref(packet)
end
----------------------------------------------------------------------------------
::clean::
local ret = FFmpeg.av_write_trailer(o_ctx[0])
assert(ret)
FFmpeg.avcodec_free_context(ffi.new('AVCodecContext*[1]', encoder))
FFmpeg.avcodec_free_context(ffi.new('AVCodecContext*[1]', decoder))
if o_ctx[0].pb then --close the output file
    FFmpeg.avio_closep(ffi.new('AVIOContext*[1]', o_ctx[0].pb))
    o_ctx[0].pb = nil
end
FFmpeg.av_frame_free(ffi.new('AVFrame*[1]', frame))
FFmpeg.av_frame_free(ffi.new('AVFrame*[1]', frame2))
FFmpeg.av_packet_free(ffi.new('AVPacket*[1]', packet))
FFmpeg.avformat_close_input(i_ct);
FFmpeg.avformat_close_input(o_ctx);
end
local scuess, reason = pcall(process, arg)
assert(scuess, reason)
