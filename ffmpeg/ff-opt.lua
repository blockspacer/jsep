local ffi, bit = require'ffi', require'bit'
ffi.cdef[[
int strcmp(const char*, const char*);
const char* strstr(const char*, const char*);
]]
local FFmpeg, arg = ...
local getopt = dofile('getopt.lua')
local iformat, icodec = {}, {} --input format, input codec
local opt = getopt(arg, 'i,c,s,m',{
    help='h', map='m', codec='c',
    acodec='c:a', vcodec='c:v',
    h=function(r, i, x)
        if not x and i < #arg then r.h = arg[i+1] end
    end,
    i=function(r)
        iformat[#r.i], r.f = r.f, nil
        icodec[#r.i], r.c = r.c, {}
    end,
})
if opt.h then dofile('ff-man.lua')(opt.h) end
opt.iformat = iformat
opt.icodec = icodec
----------------------------------------------------------------------------------
local function btest(a, b) return bit.band(a, b) ~= 0 end
----------------------------------------------------------------------------------
if opt.v then
    local log_levels={
        quiet = FFmpeg.AV_LOG_QUIET,
        panic = FFmpeg.AV_LOG_PANIC,
        fatal = FFmpeg.AV_LOG_FATAL,
        error = FFmpeg.AV_LOG_ERROR,
        warning= FFmpeg.AV_LOG_WARNING,
        info   = FFmpeg.AV_LOG_INFO,
        verbose= FFmpeg.AV_LOG_VERBOSE,
        debug  = FFmpeg.AV_LOG_DEBUG,
        trace = FFmpeg.AV_LOG_TRACE
    }
    local level
    local flags = bit.band(FFmpeg.av_log_get_flags(),
    bit.bnot(FFmpeg.AV_LOG_SKIP_REPEATED),
    bit.bnot(FFmpeg.AV_LOG_PRINT_LEVEL))
    for a in string.gmatch(opt.v, "%a+") do
        if a == 'repeat' then
            flags = bit.bor(flags, FFmpeg.AV_LOG_SKIP_REPEATED)
        elseif a== 'level' then
            flags = bit.bor(flags, FFmpeg.AV_LOG_PRINT_LEVEL)
        else
            level = log_levels[a] or tonumber(a)
        end
    end
    if not level then
        FFmpeg.av_log(nil, FFmpeg.AV_LOG_FATAL,
        "Invalid loglevel \"%s\". Possible levels are numbers or:\n", opt.v)
        for k,_ in pairs(log_levels) do
            FFmpeg.av_log(nil, FFmpeg.AV_LOG_FATAL, "\"%s\"\n", k);
        end
        os.exit(-1)
    end
    FFmpeg.av_log_set_flags(flags)
    FFmpeg.av_log_set_level(level)
    opt.v = nil
end
if not opt.hide_banner then
    local indent = '  '
    local libs= 'avutil avcodec avformat avdevice avfilter swscale swresample postproc'
    FFmpeg.av_log(nil, FFmpeg.AV_LOG_INFO, "%s Copyright (c) 2003-2018 the FFmpeg developers\n", jit and jit.version or _VERSION)
    local a = string.format("%sconfiguration: %s\n", indent, ffi.string(FFmpeg.avformat_configuration()));
    FFmpeg.av_log(nil, FFmpeg.AV_LOG_INFO, a);
    for b in string.gmatch(libs, "%a+") do
        a = FFmpeg[b..'_version']
        if a then
            a = a()
            a = string.format("%slib%-11s %2d.%3d.%3d\n", indent, b,
            bit.rshift(a,16), bit.rshift(bit.band(a,0x00FF00),8), bit.band(a,0xFF))
            FFmpeg.av_log(nil, FFmpeg.AV_LOG_INFO, a)
        end
    end
end
local function show_formats_devices(device_only, mux_only, demux_only)
    local function AV_IS_INPUT_DEVICE(category)
        return category == FFmpeg.AV_CLASS_CATEGORY_DEVICE_VIDEO_INPUT
        or category == FFmpeg.AV_CLASS_CATEGORY_DEVICE_AUDIO_INPUT
        or category == FFmpeg.AV_CLASS_CATEGORY_DEVICE_INPUT
    end
    local function AV_IS_OUTPUT_DEVICE(category)
        return category == FFmpeg.AV_CLASS_CATEGORY_DEVICE_VIDEO_OUTPUT
        or category == FFmpeg.AV_CLASS_CATEGORY_DEVICE_AUDIO_OUTPUT
        or category == FFmpeg.AV_CLASS_CATEGORY_DEVICE_OUTPUT
    end
    local function is_device(avclass)
        return avclass ~= nil and (AV_IS_INPUT_DEVICE(avclass.category) or AV_IS_OUTPUT_DEVICE(avclass.category))
    end
    io.write(device_only and "Devices:" or "File formats:",'\n',
    " D. = Demuxing supported\n",
    " .E = Muxing supported\n",
    " --\n");
    local last_name, ofmt, ifmt = "000";
    while true do
        local name, long_name, decode, encode
        if not demux_only then
            ofmt = FFmpeg.av_oformat_next(ofmt)
            while ofmt ~= nil do
                if not device_only or is_device(ofmt.priv_class) then
                    if (not name or ffi.C.strcmp(ofmt.name, name) < 0) and
                        ffi.C.strcmp(ofmt.name, last_name) > 0  then
                        name, long_name, encode = ofmt.name, ofmt.long_name, true
                    end
                end
                ofmt = FFmpeg.av_oformat_next(ofmt)
            end
        end
        if not mux_only then
            ifmt = FFmpeg.av_iformat_next(ifmt)
            while ifmt ~= nil do
                if not device_only or is_device(ifmt.priv_class) then
                    if (not name or ffi.C.strcmp(ifmt.name, name) < 0) and
                        ffi.C.strcmp(ifmt.name, last_name) > 0 then
                        name, long_name, encode = ifmt.name, ifmt.long_name, false 
                    end
                    if name and ffi.C.strcmp(ifmt.name, name) == 0 then
                        decode = true
                    end
                end
                ifmt = FFmpeg.av_iformat_next(ifmt)
            end
        end
        if not name then break end
        last_name = name

        io.write(string.format(" %s%s %-15s %s\n",
        decode and "D" or " ",
        encode and "E" or " ",
        ffi.string(name),
        long_name == nil and '' or ffi.string(long_name)))
    end
end

if opt.formats      then show_formats_devices(false)
elseif opt.muxers   then show_formats_devices(false, true)
elseif opt.demuxers then show_formats_devices(false, false, true)
elseif opt.devices  then show_formats_devices(true) end

if opt.protocols then
    local opaque = ffi.new('void*[1]')
    io.write("Supported file protocols:\n", "Input:\n")
    for name in FFmpeg.foreach('avio_enum_protocols', opaque, 0) do
        io.write('  ', ffi.string(name), '\n')
    end
    io.write("Output:\n")
    for name in FFmpeg.foreach('avio_enum_protocols', opaque, 1) do
        io.write('  ', ffi.string(name), '\n')
    end
end
if opt.pix_fmts then
    io.write("Pixel formats:\n",
    "I.... = Supported Input  format for conversion\n",
    ".O... = Supported Output format for conversion\n",
    "..H.. = Hardware accelerated format\n",
    "...P. = Paletted format\n",
    "....B = Bitstream format\n",
    "FLAGS NAME            NB_COMPONENTS BITS_PER_PIXEL\n",
    "-----\n");

    local special = type(opt.pix_fmts) == 'string' and string.len(opt.pix_fmts) > 0
    for pix_desc in FFmpeg.foreach'av_pix_fmt_desc_next' do
        local name = ffi.string(pix_desc.name)
        if not special or string.find(name, opt.pix_fmts) then
            local pix_fmt = FFmpeg.av_pix_fmt_desc_get_id(pix_desc)
            io.write(
            FFmpeg.sws_isSupportedInput (pix_fmt) ~= 0 and 'I' or '.',
            FFmpeg.sws_isSupportedOutput(pix_fmt) ~= 0 and 'O' or '.',
            btest(pix_desc.flags, FFmpeg.AV_PIX_FMT_FLAG_HWACCEL)   and 'H' or '.',
            btest(pix_desc.flags, FFmpeg.AV_PIX_FMT_FLAG_PAL)       and 'P' or '.',
            btest(pix_desc.flags, FFmpeg.AV_PIX_FMT_FLAG_BITSTREAM) and 'B' or '.',
            string.format(" %-16s       %d            %2d", name,
            pix_desc.nb_components, FFmpeg.av_get_bits_per_pixel(pix_desc)), '\n');
        end
    end
end
local function get_codecs_sorted()
    local codec_desc={}
    for desc in FFmpeg.foreach'avcodec_descriptor_next' do
        table.insert(codec_desc, desc)
    end
    table.sort(codec_desc, function(a, b)
        return a.type == b.type and ffi.C.strcmp(a.name, b.name) < 0 or a.type < b.type
    end)
    return codec_desc
end
local function get_media_type_char(type)
    if type == FFmpeg.AVMEDIA_TYPE_VIDEO then     return 'V';
    elseif type == FFmpeg.AVMEDIA_TYPE_AUDIO then return 'A';
    elseif type == FFmpeg.AVMEDIA_TYPE_DATA then  return 'D';
    elseif type == FFmpeg.AVMEDIA_TYPE_SUBTITLE then   return 'S';
    elseif type == FFmpeg.AVMEDIA_TYPE_ATTACHMENT then return 'T';
    else return '?' end
end
local function print_codecs(encoder)
    io.write(encoder and "Encoders" or "Decoders",":\n",
    " V..... = Video\n",
    " A..... = Audio\n",
    " S..... = Subtitle\n",
    " .F.... = Frame-level multithreading\n",
    " ..S... = Slice-level multithreading\n",
    " ...X.. = Codec is experimental\n",
    " ....B. = Supports draw_horiz_band\n",
    " .....D = Supports direct rendering method 1\n",
    " ------\n")
    for _, desc in ipairs(get_codecs_sorted()) do
        for codec in FFmpeg.foreach('av_codec_next', function(codec) return codec.id == desc.id and
            0 ~= (encoder and FFmpeg.av_codec_is_encoder(codec) or FFmpeg.av_codec_is_decoder(codec))
        end) do
            io.write(' ', get_media_type_char(desc.type),
            btest(codec.capabilities, FFmpeg.AV_CODEC_CAP_FRAME_THREADS) and "F" or ".",
            btest(codec.capabilities, FFmpeg.AV_CODEC_CAP_SLICE_THREADS) and "S" or ".",
            btest(codec.capabilities, FFmpeg.AV_CODEC_CAP_EXPERIMENTAL)  and "X" or ".",
            btest(codec.capabilities, FFmpeg.AV_CODEC_CAP_DRAW_HORIZ_BAND)and "B" or ".",
            btest(codec.capabilities, FFmpeg.AV_CODEC_CAP_DR1)           and "D" or ".",
            string.format(" %-20s ", ffi.string(codec.name)),
            codec.long_name == nil and '' or ffi.string(codec.long_name),
            ffi.C.strcmp(codec.name, desc.name) == 0 and '\n' or
            string.format(" (codec %s)\n", ffi.string(desc.name)))
        end
    end
end

if opt.codecs then
    -- print decoders/encoders when there's more than one or their
    -- names are different from codec name
    local function print_next_codec(desc, encoder)
        local names, found={},nil
        for codec in FFmpeg.foreach('av_codec_next', function(codec) return codec.id == desc.id and
            0 ~= (encoder and FFmpeg.av_codec_is_encoder(codec) or FFmpeg.av_codec_is_decoder(codec))
        end) do
            if not found and ffi.C.strcmp(codec.name, desc.name) ~= 0 then found = true end
            table.insert(names, ffi.string(codec.name))
        end
        if found then 
            return string.format(" (%s: %s)", encoder and "encoders" or "decoders", table.concat(names,' '))
        end
        return ''
    end
    io.write("Codecs:\n",
    " D..... = Decoding supported\n",
    " .E.... = Encoding supported\n",
    " ..V... = Video codec\n",
    " ..A... = Audio codec\n",
    " ..S... = Subtitle codec\n",
    " ...I.. = Intra frame-only codec\n",
    " ....L. = Lossy compression\n",
    " .....S = Lossless compression\n",
    " -------\n");
    local special = type(opt.codecs) == 'string' and string.len(opt.codecs) > 0
    for _, desc in ipairs(get_codecs_sorted()) do
        local name = ffi.string(desc.name)
        if ffi.C.strstr(desc.name, "_deprecated") == nil and
            (not special or string.find(name, opt.codecs)) then
            io.write(' ',
            FFmpeg.avcodec_find_decoder(desc.id) ~= nil and "D" or ".",
            FFmpeg.avcodec_find_encoder(desc.id) ~= nil and "E" or ".",
            get_media_type_char(desc.type),
            btest(desc.props, FFmpeg.AV_CODEC_PROP_INTRA_ONLY) and "I" or ".",
            btest(desc.props, FFmpeg.AV_CODEC_PROP_LOSSY)      and "L" or ".",
            btest(desc.props, FFmpeg.AV_CODEC_PROP_LOSSLESS)   and "S" or ".",
            string.format(" %-20s ", name),
            desc.long_name == nil and '' or ffi.string(desc.long_name),
            print_next_codec(desc, false),
            print_next_codec(desc, true),
            '\n')
        end
    end
    opt.codecs = nil
elseif opt.decoders then
    print_codecs(false)
elseif opt.encoders then
    print_codecs(true)
end
----------------------------------------------------------------------------------
if #opt.i ==0 then os.exit(0) end
if opt.pix_fmt then
    local pix_fmt = FFmpeg.av_get_pix_fmt(opt.pix_fmt)
    assert(pix_fmt ~= FFmpeg.AV_PIX_FMT_NONE, opt.pix_fmt)
    opt.pix_fmt = pix_fmt
end
if opt.codec.v then
    local codec = FFmpeg.avcodec_find_encoder_by_name(opt.codec.v)
    assert(codec ~= nil, opt.codec.v)
    opt.codec.v = codec.id
end
for i,f in ipairs(opt.iformat) do
    local fmt = FFmpeg.av_find_input_format(f)
    FFmpeg.assert(fmt, f)
    opt.iformat[i] = fmt
end
local function filter_codec_opts(codec_id, fmtcxt, stream, codec)
    local ret = ffi.new('AVDictionary*[1]')
    ffi.gc(ret, FFmpeg.av_dict_free)
    local cc = ffi.new('void*[1]', ffi.cast('void*', FFmpeg.avcodec_get_class()))
    local flags = fmtcxt.oformat ~= nil and FFmpeg.AV_OPT_FLAG_ENCODING_PARAM or FFmpeg.AV_OPT_FLAG_DECODING_PARAM
    if not codec then
        if fmtcxt.oformat ~= nil then
            codec =  FFmpeg.avcodec_find_encoder(codec_id)
        else
            codec =  FFmpeg.avcodec_find_decoder(codec_id)
        end
    end
    local priv_class
    if codec ~= nil and codec.priv_class ~= nil then
        priv_class = ffi.new('void*[1]', ffi.cast('void*', codec.priv_class))
    end
    local prefix
    if stream.codecpar.codec_type == FFmpeg.AVMEDIA_TYPE_VIDEO then
        prefix  = 'v';
        flags  = bit.bor(flags, FFmpeg.AV_OPT_FLAG_VIDEO_PARAM)
    elseif  stream.codecpar.codec_type == FFmpeg.AVMEDIA_TYPE_AUDIO then
        prefix  = 'a';
        flags  = bit.bor(flags, FFmpeg.AV_OPT_FLAG_AUDIO_PARAM)
    elseif  stream.codecpar.codec_type == FFmpeg.AVMEDIA_TYPE_SUBTITLE then
        prefix  = 's';
        flags  = bit.bor(flags, FFmpeg.AV_OPT_FLAG_SUBTITLE_PARAM)
    end
    for k, v in pairs(opt) do
        if type(k) ~= 'string' or type(v) ~= 'string' then goto continue end
        local p = string.find(k, ':', 1, true)
        -- check stream specification in opt name
        if p then
            local s = string.sub(k, p+1)
            local r = FFmpeg.avformat_match_stream_specifier(fmtcxt, stream, s);
            if r < 0 then
                FFmpeg.av_log(fmtcxt, FFmpeg.AV_LOG_ERROR, "Invalid stream specifier: %s\n", s);
                os.exit(-1)
            elseif r == 0 then
                goto continue --skip
            else
                k = string.sub(k, 1, p-1)
            end
        end
        if nil ~= FFmpeg.av_opt_find(cc, k, nil, flags, FFmpeg.AV_OPT_SEARCH_FAKE_OBJ) or
            (priv_class and 
            nil ~= FFmpeg.av_opt_find(priv_class, k, nil, flags, FFmpeg.AV_OPT_SEARCH_FAKE_OBJ)) then
            FFmpeg.av_dict_set(ret, k, v, 0)
        elseif string.sub(k, 1, 1) == prefix and
            nil ~= FFmpeg.av_opt_find(cc, string.sub(k, 2), nil, flags, FFmpeg.AV_OPT_SEARCH_FAKE_OBJ) then
            FFmpeg.av_dict_set(ret, string.sub(k, 2), v, 0)
        end
        ::continue::
    end
    return ret
end
opt.codec_opts = filter_codec_opts
return opt
