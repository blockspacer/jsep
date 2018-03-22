using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

///<summary> ģ�� WEBRTC 1.0 ��׼ʵ��1��1��ͨ������
///</summary>
public static class JSEP 
{
    ///<summary> JSEP �ӿ����,�������ж��Ƿ�߱�JSEP����
    ///</summary>
    public static readonly IntPtr JSEP_API;

    ///<summary>RTCSocket �¼�</summary>
    public enum RTCSocketEvent
    {
        ///<summary>�յ���Ϣ</summary>
        Message     = 0,

        ///<summary>״̬�ı�,��Ӧ����Ϊ��״̬</summary>
        StateChange = 1,

        ///<summary>��ȡ�µı��غ�ѡ��ַ, ��Ӧ����Ϊ��ѡ��ַ</summary>
        IceCandidate= 2,
    };

    ///<summary> RTCSocket �ͻ���
    ///</summary>
    public class RTCSocket : IDisposable
    {
        internal IntPtr iface;
        internal RTCSocket() {}
       
        ///<summary> ����������Դ
        ///</summary>
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        ///<summary> ����������Դ
        ///</summary>
        ///<param name="isDisposing"> �Ƿ�����������</param>
        protected virtual void Dispose(bool isDisposing)
        {
            if (iface != IntPtr.Zero){
                OBSERVERS[(int)JSEP._CloseSocket(iface)] = null;
                SOCKETS.Remove(iface);
                iface = IntPtr.Zero;
            }
        }

        ///<summary> �Զ�����
        ///</summary>
        ~RTCSocket()
        {
            Dispose(false);
        }

        ///<summary> ��������
        ///</summary>
        ///<param name="message"> ��Ϣ </param>
        ///<returns> �ɹ�����0 </returns>
        public int Send(string message)
        {
            byte[] buffer = Encoding.UTF8.GetBytes(message);
            return JSEP._SendSocket(iface, buffer, buffer.Length, 0);
        }
    }

    ///<summary> RTCSocket�¼������ص�����
    ///</summary>
    ///<param name="rtcsocket"> RTCSocket </param>
    ///<param name="message"> �յ���Ϣ </param>
    ///<param name="eventId"> ��Ӧ�¼� </param>
    public delegate void RTCSocketObserver(RTCSocket rtcsocket, string message, RTCSocketEvent eventId);

    ///<summary> WebSocket �ͻ���
    ///</summary>
    public class WebSocket : RTCSocket 
    {
        private WebSocket() {}

        private static readonly _RTCSocketEventHandler WebSocketCallback = (user, iface, utf8, len, eventId) =>{
            RTCSocketObserver observer = (RTCSocketObserver)OBSERVERS[(int)user];
            if (observer != null){
                string txt = null;
                if (utf8 != null) txt = Encoding.UTF8.GetString(utf8);
                RTCSocket rtcsocket;
                if (!SOCKETS.TryGetValue(iface, out rtcsocket)){
                    rtcsocket = new WebSocket();
                    rtcsocket.iface = iface;
                    SOCKETS.Add(iface, rtcsocket);
                }
                observer(rtcsocket, txt, (RTCSocketEvent)eventId);
            }
        };

        ///<summary> ���� WebSocket �ͻ��˲����ӵ�������
        /// WebSocket �ǿɿ���TCP����
        ///</summary>
        ///<param name="wsURL"> ��������ַ</param>
        ///<param name="rtcWebSocketInit"> ���ò���</param>
        ///<param name="observer"> �¼�������</param>
        static public WebSocket Connect(string wsURL, string rtcWebSocketInit, RTCSocketObserver observer)
        {
            if (observer == null)
                throw new ArgumentNullException();

            int index = 1;
            while (index < OBSERVERS.Length && OBSERVERS[index] != null)
                ++index;
            if (index == OBSERVERS.Length)
                throw new OverflowException();

            OBSERVERS[index] = observer;
            IntPtr iface = JSEP._CreateWebSocket(wsURL, rtcWebSocketInit, (IntPtr)index, WebSocketCallback);
            if (iface == IntPtr.Zero) {
                OBSERVERS[index] = null;
                throw new ArgumentException();
            }
            WebSocket self = new WebSocket();
            SOCKETS.Add(iface, self);
            self.iface = iface;
            return self;
        }

        ///<summary> ���� WebSocket �������������ͻ������� 
        /// WebSocket �ǿɿ���TCP����
        ///</summary>
        ///<param name="wsURL"> ��������ַ</param>
        ///<param name="rtcWebSocketInit"> ���ò���</param>
        ///<param name="observer"> �¼�������</param>
        static public WebSocket Listen(string wsURL, string rtcWebSocketInit, RTCSocketObserver observer)
        {
            if (observer == null)
                throw new ArgumentNullException();

            int index = 1;
            while (index < OBSERVERS.Length && OBSERVERS[index] != null)
                ++index;
            if (index == OBSERVERS.Length)
                throw new OverflowException();

            OBSERVERS[index] = observer;
            IntPtr iface = JSEP._CreateWebSocketServer(wsURL, rtcWebSocketInit, (IntPtr)index, WebSocketCallback);
            if (iface == IntPtr.Zero) {
                OBSERVERS[index] = null;
                throw new ArgumentException();
            }
            WebSocket self = new WebSocket();
            SOCKETS.Add(iface, self);
            self.iface = iface;
            return self;
        }
    }

    ///<summary> IceSocket �ͻ���
    ///</summary>
    public class IceSocket : RTCSocket 
    {
        private IceSocket() {}

        private static readonly _RTCSocketEventHandler IceSocketCallback = (user, iface, utf8, len, eventId) =>{
            RTCSocketObserver observer = (RTCSocketObserver)OBSERVERS[(int)user];
            if (observer != null){
                string txt = null;
                if (utf8 != null) txt = Encoding.UTF8.GetString(utf8);
                RTCSocket rtcsocket;
                if (!SOCKETS.TryGetValue(iface, out rtcsocket)){
                    rtcsocket = new IceSocket();
                    rtcsocket.iface = iface;
                    SOCKETS.Add(iface, rtcsocket);
                }
                observer(rtcsocket, txt, (RTCSocketEvent)eventId);
            }
        };

        ///<summary> ���� IceSocket
        /// IceSocket �ǿɿ���TCP����
        ///</summary>
        ///<param name="rtcConfiguration"> ��������ַ</param>
        ///<param name="rtcIceParameters"> ��ѡ����Դ��ʶ</param>
        ///<param name="isCaller"> �Ƿ�Ϊ����</param>
        ///<param name="observer"> �¼�������</param>
        static public IceSocket Connect(string rtcConfiguration, string rtcIceParameters, bool isCaller, RTCSocketObserver observer)
        {
            if (observer == null)
                throw new ArgumentNullException();

            int index = 1;
            while (index < OBSERVERS.Length && OBSERVERS[index] != null)
                ++index;
            if (index == OBSERVERS.Length)
                throw new OverflowException();

            OBSERVERS[index] = observer;
            IntPtr iface = JSEP._CreateIceSocket(rtcConfiguration, rtcIceParameters, isCaller, (IntPtr)index, IceSocketCallback);
            if (iface == IntPtr.Zero) {
                OBSERVERS[index] = null;
                throw new ArgumentException();
            }
            IceSocket self = new IceSocket();
            SOCKETS.Add(iface, self);
            self.iface = iface;
            return self;
        }

        ///<summary> ��ӶԶ˵�ICE��ַ
        ///</summary>
        ///<param name="candidate">�Զ˵�ַ</param>
        ///<returns> �ɹ�����0 </returns>
        public int AddRemoteCandidate(string candidate)
        {
            return JSEP._AddSocketIceCandidate(iface, candidate);
        }

        ///<summary> ���öԶ˵�ICE����
        ///</summary>
        ///<param name="rtcIceParameters">�Զ˲���, @see RTCIceParameters</param>
        ///<returns> �ɹ�����0 </returns>
        public int SetRemoteParameters(string rtcIceParameters)
        {
            return JSEP._SetSocketIceParameters(iface, rtcIceParameters);
        }
    }
    
    ///<summary> ʵʱͨ�����¼������ص�����
    ///</summary>
    ///<param name="eventId"> Ϊ�¼���,enum JsepEvent���� </param>
    ///<param name="json"> JSON��ʽ�ַ���</param>
    public delegate void RTCSessionObserver(RTCSessionEvent eventId, string json);

    ///<summary> ͨ���еĴ���ֵ</summary>
    public enum RTCSessionError
    {
        ///<summary>��Ч����,δ֪����</summary>
        InvalidOperation    = -1,

        ///<summary>�Ƿ��Ĳ���ʹ��</summary>
        InvalidArgument     = -2,

        ///<summary>û����Ƶ��</summary>
        MissingVideoTrack   = -3,

        ///<summary>�汾��ƥ��</summary>
        MismatcheaVersion   = -4,
    };

    ///<summary> ͨ���еĻ����¼�,Я����JSON��ʽ����</summary>
    public enum RTCSessionEvent
    {
        ///<summary> ��Ҫ��������Э��
        /// ͨ�����JSEP_CreateOffer(),���ظ�Э������
        ///</summary>
        RenegotiationNeeded = 1,

        ///<summary> ����SDP�ɹ�
        /// {JsepSdpType:"%s",JsepSdp:{...}}
        /// Э��ʱ,��Ҫ����JSEP_SetLocalDescription()���ø�SDP
        ///</summary>
        CreateDescriptionSuccess,

        ///<summary> ����SDPʧ��
        /// {JsepSdpType:"%s",JsepReason:"%s"}
        ///</summary>
        CreateDescriptionFailure,

        ///<summary> ����SDP�ɹ�
        /// {JsepSdpType:"%s",JsepSdpLocation:"%s",JsepSdp:{...}}
        /// ����"remote","offer",����Ҫ������Ӧ�� answer SDP,�������Զ�
        ///</summary>
        SetDescriptionSuccess,

        ///<summary> ����SDPʧ��
        /// {JsepSdpType:"%s",JsepSdpLocation:"%s",JsepReason:"%s"}
        ///</summary>
        SetDescriptionFailure,

        ///<summary> �µ�ICE��ѡ��ַ
        /// {JsepIceCandidate:{...}}
        /// Э��ʱ,����ICE��ַͨ������֪ͨ�Զ�
        ///</summary>
        IceCandidate,

        ///<summary> ICE����״̬�ı�
        /// {JsepIceConnectionState:"%s"}
        ///</summary>
        IceConnectionStateChange,

        ///<summary> Э��״̬�ı�
        /// {JsepSignalingState:"%s"}
        ///</summary>
        SignalingChange,

        ///<summary> ����ӶԶ�ý����
        /// {JsepStreamId:"%s",JsepAudioTrackCount:%d,JsepVideoTrackCount:%d}
        /// ��Ҫ����JSEP_PublishStream(),��������,�Ա���Zmf�Խ�
        ///</summary>
        AddRemoteStream,

        ///<summary> ���Ƴ��Զ�ý����
        /// {JsepStreamId:"%s"}
        ///</summary>
        RemoveRemoteStream,

        ///<summary> DTMF �����ѷ���
        /// {JsepTone:"%s"}
        ///</summary>
        ToneChange,

        ///<summary> �յ�ͳ�Ʊ���
        /// {JsepStatsType:"%s",JsepStatsId:"%s",JsepStatsTimestamp:%f,JsepStats:{...}}
        ///</summary>
        StatsReport,

        ///<summary> �Ѵ�����ͨ��,���շ���Ϣ
        /// {JsepChannelId:"%s",JsepChannelConfig:{...}}
        ///</summary>
        DataChannelOpen,

        ///<summary> �յ�����
        /// {JsepChannelId:"%s",JsepMessage:"%s"}
        ///</summary>
        DataChannelMessage,

        ///<summary> �ѹر�����ͨ��
        /// �ѹر�����ͨ��,������ʹ��JSEP_SendMessage����
        /// {JsepChannelId:"%s",JsepReason:"%s"}
        ///</summary>
        DataChannelClose,
    }

    ///<summary> P2Pͨ��ʵ��
    ///</summary>
    public class RTCPeerConnection : IDisposable
    {
        private IntPtr iface;

        ///<summary> ����P2Pͨ��ʵ��
        ///</summary>
        ///<param name="config"> ���ò���,JSON��ʽ. ����{'iceServers': [{'url': url}]}</param>
        ///<param name="zmfAudioPump"> �Ƿ�ʹ�����õ�'ȫ����Ƶ��',�����ⲿ��Ƶ���Խ�, @see JSEP_AUDIO_PUMP</param>
        ///<param name="isCaller"> �Ƿ�Ϊ����</param>
        ///<param name="observer"> �¼�������</param>
        ///<remarks>
        /// config ֧�ֵ����ò���
        ///     - STRUN ��������ַ
        ///         iceServers: [{
        ///             urls:'url'
        ///             username:''
        ///             credential:''
        ///         }]
        ///     - bundlePolicy:'balanced'
        ///     - rtcpMuxPolicy:'require' | 'negotiate'
        ///     - iceTransportPolicy:'all'
        ///     - zmfAudioPump: false //ȫ������,ʹ��JSEP���õ���Ƶ����,�����ⲿ��Ƶ���Խ�
        ///
        /// constraints ֧�ֵ�����
        ///     - googIPv6: true
        ///     - googDscp: true
        ///     - DtlsSrtpKeyAgreement: true
        ///     - RtpDataChannels: false
        ///     - googSuspendBelowMinBitrate: true
        ///     - googNumUnsignalledRecvStreams: 20
        ///     - googScreencastMinBitrate:int
        ///     - googHighStartBitrate:int
        ///     - googHighBitrate:true
        ///     - googVeryHighBitrate:true
        ///     - googCombinedAudioVideoBwe:true
        ///</remarks>
        public RTCPeerConnection(string config, bool zmfAudioPump, bool isCaller, RTCSessionObserver observer)
        {
            if (observer == null)
                throw new ArgumentNullException();

            int index = 1;
            while (index < OBSERVERS.Length && OBSERVERS[index] != null)
                ++index;
            if (index == OBSERVERS.Length)
                throw new OverflowException();

            OBSERVERS[index] = observer;
            iface = JSEP._CreatePeerConnection(config, zmfAudioPump, isCaller, (IntPtr)index, RTCSessionCallback);
            if (iface == IntPtr.Zero) {
                OBSERVERS[index] = null;
                throw new ArgumentException();
            }
        }

        ///<summary> ����������Դ
        ///</summary>
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        ///<summary> ����������Դ
        ///</summary>
        ///<param name="isDisposing"> �Ƿ�����������</param>
        protected virtual void Dispose(bool isDisposing)
        {
            if (iface != IntPtr.Zero)
                OBSERVERS[(int)JSEP._ReleasePeerConnection(iface)] = null;
            iface = IntPtr.Zero;
        }

        ///<summary> �Զ�����
        ///</summary>
        ~RTCPeerConnection()
        {
            Dispose(false);
        }

        ///<summary> �����Զ�������
        ///</summary>
        ///<param name="channelId"> ͨ��ID</param>
        ///<param name="message"> ��Ϣ </param>
        ///<returns> �ɹ�����0 </returns>
        public int SendMessage(string channelId, string message)
        {
            byte[] buffer = Encoding.UTF8.GetBytes(message);
            return JSEP._SendMessage(iface, channelId, buffer, buffer.Length);
        }

        ///<summary> ��ӶԶ˵�ICE��ѡ��ַ
        ///</summary>
        ///<param name="candidate"> ��ѡ��ַ,ͨ���Զ��� JsepEventIceCandidate �¼���JsepIceCandidate�ֶλ��</param>
        ///<returns> �ɹ�����0 </returns>
        public int AddIceCandidate(string candidate)
        {
            return JSEP._AddIceCandidate(iface, candidate);
        }

        ///<summary> ��ӱ���ý����
        ///</summary>
        ///<param name="streamId"> ý����ID</param>
        ///<param name="bAudio"> �Ƿ���Ƶ</param>
        ///<param name="bVideo"> �Ƿ���Ƶ</param>
        ///<param name="constraints"> ý���������Ʋ���,JSON��ʽ</param>
        ///<returns> �ɹ�����0,�ɹ�����0, bAudio, bVideo�ֱ�ָʾ�Ƿ�����Ƶ </returns>
        ///<remarks>
        /// ����ý����ͨ����Э��ǰ���,�Զ�ý�������Զ���ӵ�.
        /// streamId Ҫ��֤Ψһ��,ͨ�����˻�ID.
        ///
        /// constraints Ŀǰ֧��
        /// - ��Ƶ
        ///   video: {
        ///     zmfCapture:Id0      //ZMF�ɼ�ԴID
        ///     zmfRender:Id0       //ZMF��������ȾԴID,ת��Ϊ�ɼ�
        ///   }
        /// - ��Ƶ
        ///   audio: {
        ///     DTMF:false          //��ʹ��JSEP_InsertDtmf����,�ɽ�ʡЩ�ڴ�.
        ///   }
        ///</remarks>
        public int AddLocalStream (string streamId, ref bool bAudio, ref bool bVideo, string constraints)
        {
            return JSEP._AddLocalStream(iface, streamId, ref bAudio, ref bVideo, constraints);
        }

        ///<summary> �ر�����ͨ��
        ///</summary>
        ///<param name="channelId"> ͨ��ID</param>
        public void CloseDataChannel (string channelId)
        {
            JSEP._CloseDataChannel(iface, channelId);
        }

        ///<summary> ��̬��������ͨ��
        ///</summary>
        ///<param name="channelId"> ͨ��ID</param> 
        ///<param name="constraints"> ���ò���</param> 
        ///<returns> �ɹ�����0</returns>
        ///<remarks>
        /// ֧�ֵ����ò���
        /// - ordered:true,       //�Ƿ�֤����,Ĭ��true
        /// - maxPacketLifeTime:0,//������ʱ,�����ط�.Ĭ��-1,ʼ���ط�
        /// - maxRetransmits:0,   //��������,�����ط�,Ĭ��-1,,ʼ���ط�
        /// - negotiated:false,   //�Ƿ����ϲ�Ӧ�ø���Э�̽�������,
        ///                         ��������DataChannelOpen�¼�.Ĭ��false���ڲ��Զ����
        /// - protocol:'',        //�Զ�����ϲ�Ӧ��Э����,Ĭ�Ͽ�
        ///</remarks>
        public int CreateDataChannel (string channelId, string constraints)
        {
            return JSEP._CreateDataChannel(iface, channelId, constraints);
        }

        ///<summary> ���� answer SDP
        ///</summary>
        ///<param name="constraints"> ��ѡ��SDP�����Ʋ���.JSON��ʽ</param>
        ///<seealso cref="CreateOffer"/>
        ///<returns> ��Ч������������-1,��֮�첽����,�����JsepEventCreateDescription �¼�����</returns>
        public int CreateAnswer (string constraints)
        {
            return JSEP._CreateAnswer(iface, constraints);
        }

        ///<summary> ����offer SDP
        ///</summary>
        ///<param name="constraints"> ��ѡ��SDP�����Ʋ���.JSON��ʽ</param>
        ///<returns> ��Ч������������-1,��֮�첽����,�����JsepEventCreateDescription �¼�����</returns>
        ///<remarks>
        /// constraints ֧����������
        /// - OfferToReceiveAudio: true
        /// - OfferToReceiveVideo: true
        /// - VoiceActivityDetection: true
        /// - IceRestart: false
        /// - googUseRtpMUX: true
        ///</remarks>
        public int CreateOffer (string constraints)
        {
            return JSEP._CreateOffer(iface, constraints);
        }

        ///<summary> ��ȡͳ��
        ///</summary>
        ///<param name="statsType"> ͳ�����,""��null��ʾ��ȡ����ͳ��</param>
        ///<param name="bDebug"> �Ƿ����ϸ�ĵ��Լ���</param>
        ///<returns> �ɹ�����0, ������ JsepEventStatsReport �¼�</returns>
        public int GetStats (string statsType, bool bDebug)
        {
            return JSEP._GetStats(iface, statsType, bDebug);
        }

        ///<summary> ��ʼ/ֹͣ��¼ͨ���¼�
        ///</summary>
        ///<param name="filename"> ��־�ļ���,�ջ�null��ֹͣ��¼</param>
        ///<param name="max_size_mb"> ��־�ļ������СMB</param>
        ///<returns> �ɹ�����0</returns>
        public int LogRtcEvent (string filename, int max_size_mb)
        {
            return JSEP._LogRtcEvent(iface, Encoding.UTF8.GetBytes(filename), max_size_mb);
        }

        ///<summary> �ֹ����÷�������
        ///</summary>
        ///<param name="current_bitrate_bps"> ��ǰ����,��������0,������</param>
        ///<param name="max_bitrate_bps"> ������������,��������0,������</param>
        ///<param name="min_bitrate_bps"> �������С����,��������0,������</param>
        ///<returns> �ɹ�����0</returns>
        public int SetBitrate (int current_bitrate_bps, int max_bitrate_bps, int min_bitrate_bps)
        {
            return JSEP._SetBitrate(iface, current_bitrate_bps, max_bitrate_bps, min_bitrate_bps);
        }

        ///<summary> ����DTMF
        ///</summary>
        ///<param name="tones"> DMTF������,0-9,A-D��a-d,#,*. �����޷�ʶ����ַ�</param>
        ///<param name="duration"> ÿ�������ĳ�������MS,���ܳ���6000��С��70</param>
        ///<param name="inter_tone_gap"> �������,��������Ϊ50ms,��Ӧ�����ܶ�</param>
        ///<returns> �ɹ�����0,������ JsepEventToneChange �¼� </returns>
        ///<remarks>
        /// ���л�����DTMF ����.
        /// ','��ʾ�ӳ�2�봦����һ���ַ�
        /// ������ʱ,�ϴ���������,��֮ǰ�����񽫱�ȡ��
        ///</remarks>
        public int InsertDtmf (string tones, int duration, int inter_tone_gap)
        {
            return JSEP._InsertDtmf(iface, tones, duration, inter_tone_gap);
        }

        ///<summary> ��ý����������ZMF��
        ///</summary>
        ///<param name="streamId"> �Զ�ý����ID</param>
        ///<param name="renderOrCapturerBits"> ��λ����Ƶ�����Ϊ0-ZMF��Ⱦ,1-ZMF��ͷ</param>
        ///<param name="videoTrackMask"> �ϸ�����renderOrCapturerBits�е���Чλ����</param>
        ///<returns> �ɹ�����0 </returns>
        public int PublishRemoteStream(string streamId, int renderOrCapturerBits, int videoTrackMask)
        {
            return JSEP._PublishRemoteStream(iface, streamId, renderOrCapturerBits, videoTrackMask);
        }

        ///<summary> �Ƴ�����ý����
        ///</summary>
        ///<param name="streamId"> ý����ID,��""��null���Ƴ����б�����</param>
        public void RemoveLocalStream (string streamId)
        {
            JSEP._RemoveLocalStream(iface, streamId);
        }

        ///<summary> ���ñ���(����)��SDP
        ///</summary>
        ///<param name="desc"> desc SDP��JSON��,ͨ���Զ��� JsepEventCreateDescription �¼���JsepSdp���</param>
        ///<returns> ��Ч������������-1,��֮�첽����,�����JsepEventSetDescription �¼����� </returns>
        public int SetLocalDescription (string desc)
        {
            return JSEP._SetLocalDescription(iface, desc);
        }

        ///<summary> ���öԶ�(����)��SDP
        ///</summary>
        ///<param name="desc"> SDP��JSON��,ͨ���Զ��� JsepEventCreateDescription �¼���JsepSdp���</param>
        ///<returns> ��Ч������������-1,��֮�첽����,�����JsepEventSetDescription �¼����� </returns>
        public int SetRemoteDescription (string desc)
        {
            return JSEP._SetRemoteDescription(iface, desc);
        }
    }

    ///<summary> ��ʼ/ֹͣ��¼��Ƶ�������
    ///</summary>
    ///<param name="filename"> ��־�ļ���,�ջ�null��ֹͣ��¼</param>
    ///<param name="max_size_mb"> ��־�ļ������СMB</param>
    ///<returns> �ɹ�����0</returns>
    public static int DumpAudioProcessing(string filename, int max_size_mb)
    {
        return _DumpAudioProcessing(Encoding.UTF8.GetBytes(filename), max_size_mb);
    }

    ///<summary>
    /// �ڲ�ȫ����Ƶ����ID,���������/����豸.
    /// �ⲿ��֮�Խ�ʱ,��ʹ�ø�ID
    ///</summary>
    public const string JSEP_AUDIO_PUMP = " jsepAudioPump";

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
#region RTCSessionEvent�¼�Я��JSON�������ֶ���
    ///<summary> �¼�����, ��������:enum JespEvent </summary>
    public const string  JespEvent  = "JespEvent";

    ///<summary> SDP����, ��������:UTF8�ַ���, ��ѡֵ
    /// - "offer" ��������
    /// - "answer" Ӧ������
    ///</summary>
    public const string  JsepSdpType = "JsepSdpType";

    ///<summary> SDPλ��, ��������:UTF8�ַ���, ��ѡֵ
    /// - "local" ���ؽ��ղ�
    /// - "remote" ��Ӧ���Ͳ�
    ///</summary>
    public const string  JsepSdpLocation  = "JsepSdpLocation";

    ///<summary> SDP����,��������: JSON�� </summary>
    public const string  JsepSdp = "JsepSdp";

    ///<summary> ԭ��, ��������: UTF8�ַ��� </summary>
    public const string  JsepReason = "JsepReason";

    ///<summary> ICE��ַ, ��������: JSON�� </summary>
    public const string  JsepIceCandidate = "JsepIceCandidate";

    ///<summary> ICE����״̬, ��������: UTF8�ַ���, ��ѡֵ
    /// - "new" ��ʼ״̬,
    /// - "checking" ̽�����״̬
    /// - "connected" �ɹ�����
    /// - "completed" ̽�����
    /// - "disconnected" �Ͽ�����
    /// - "failed" ���ӹ�����ʧ��,���ɻָ�
    /// - "closed", �ر�����
    /// ֻ����connected��completed״̬��,�ſɽ���ͨ�Ź���.
    ///</summary>
    public const string  JsepIceConnectionState = "JsepIceConnectionState";

    ///<summary> Э��״̬, ��������: UTF8�ַ���, ��ѡֵ
    /// - "stable", û�н���Э�̵��ȶ�״̬.
    /// - "have-local-offer", �ѵ�SetLocalDescription(offer),������ղ�����SetRemoteDescription(answer)
    /// - "have-remote-offer",�ѵ�SetRemoteDescription(offer),���봴����Ӧ��SetLocalDescription(answer)
    /// - "have-remote-pranswer",�ѵ�SetRemoteDescription(answer),û��SetLocalDescription(offer)״̬
    /// - "have-local-pranswer",�ѵ�SetLocalDescription(answer),û��SetRemoteDescription(offer)״̬
    /// - "closed",�����ѹرյ�״̬.
    ///</summary>
    public const string JsepSignalingState  = "JsepSignalingState";

    ///<summary> ý����ID, ��������: UTF8�ַ��� </summary>
    public const string  JsepStreamId = "JsepStreamId";

    ///<summary> ý��������Ƶ�������, ��������: int </summary>
    public const string  JsepAudioTrackCount = "JsepAudioTrackCount";

    ///<summary> ý��������Ƶ�������, ��������: int </summary>
    public const string  JsepVideoTrackCount = "JsepVideoTrackCount";

    ///<summary> ����ͨ��ID, ��������: UTF8�ַ��� </summary>
    public const string  JsepChannelId = "JsepChannelId";

    ///<summary> ����ͨ������, ��������: JSON�� </summary>
    public const string  JsepChannelConfig = "JsepChannelConfig";

    ///<summary> ��Ϣ����, ��������: UTF8�ַ��� </summary>
    public const string  JsepMessage = "JsepMessage";

    ///<summary> DTMF �ѷ�������, ��������: UTF8�ַ���
    /// ��Ϊ""��nullptr���ʾ�ѷ������
    ///</summary>
    public const string  JsepTone = "JsepTone";

    ///<summary> ͳ������, ��������: UTF8�ַ���, ��ѡֵ
    /// - googLibjingleSession  ȫ�ֻỰ
    /// - transport             �����
    /// - VideoBwe              ��Ƶ�������
    /// - remoteSsrc            �Զ�RTP��
    /// - ssrc                  RTP��
    /// - googTrack             ý����
    /// - localcandidate        ����ICE
    /// - remotecandidate       �Զ�ICE
    /// - googComponent
    /// - googCandidatePair
    /// - googCertificate
    /// - datachannel           ����ͨ��
    ///</summary>
    public const string  JsepStatsType = "JsepStatsType";

    ///<summary> ͳ��ID, ��������: UTF8�ַ��� </summary>
    public const string  JsepStatsId = "JsepStatsId";

    ///<summary> ͳ��ʱ���, ��������: double </summary>
    public const string  JsepStatsTimestamp = "JsepStatsTimestamp";

    ///<summary> ͳ��ֵ, ��������: JSON�� </summary>
    public const string  JsepStats = "JsepStats";
#endregion
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
#region API�ĺ���ָ��
    private static readonly PFN_AddIceCandidate _AddIceCandidate;
    private static readonly PFN_AddLocalStream _AddLocalStream;
    private static readonly PFN_CloseDataChannel _CloseDataChannel;
    private static readonly PFN_CreateDataChannel _CreateDataChannel;
    private static readonly PFN_CreateAnswer _CreateAnswer;
    private static readonly PFN_CreateOffer _CreateOffer;
    private static readonly PFN_GetStats _GetStats;
    private static readonly PFN_InsertDtmf _InsertDtmf;
    private static readonly PFN_PublishRemoteStream _PublishRemoteStream;
    private static readonly PFN_RemoveLocalStream _RemoveLocalStream;
    private static readonly PFN_SetLocalDescription _SetLocalDescription;
    private static readonly PFN_SetRemoteDescription _SetRemoteDescription;
    private static readonly PFN_CreatePeerConnection _CreatePeerConnection;
    private static readonly PFN_ReleasePeerConnection _ReleasePeerConnection;
    private static readonly PFN_SendMessage _SendMessage;
    private static readonly PFN_LogRtcEvent _LogRtcEvent;
    private static readonly PFN_SetBitrate _SetBitrate;
    private static readonly PFN_DumpAudioProcessing _DumpAudioProcessing;

    private static readonly PFN_CreateWebSocket _CreateWebSocket;
    private static readonly PFN_CreateWebSocketServer _CreateWebSocketServer;
    private static readonly PFN_CreateIceSocket _CreateIceSocket;
    private static readonly PFN_CloseSocket _CloseSocket;
    private static readonly PFN_SendSocket _SendSocket;
    private static readonly PFN_AddSocketIceCandidate _AddSocketIceCandidate;
    private static readonly PFN_SetSocketIceParameters _SetSocketIceParameters;
#endregion
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
#region �ӿڵĴ�������
    [UnmanagedFunctionPointer(JSEP_CALL,CharSet = CharSet.Ansi)]
    private delegate int PFN_AddIceCandidate(IntPtr iface, string candidate);

    [UnmanagedFunctionPointer(JSEP_CALL,CharSet = CharSet.Ansi)]
    private delegate int PFN_AddLocalStream (IntPtr iface, string streamId, ref bool bAudio, ref bool bVideo, string constraints);

    [UnmanagedFunctionPointer(JSEP_CALL,CharSet = CharSet.Ansi)]
    private delegate void PFN_CloseDataChannel (IntPtr iface, string channelId);

    [UnmanagedFunctionPointer(JSEP_CALL,CharSet = CharSet.Ansi)]
    private delegate IntPtr PFN_CreatePeerConnection(string config, bool zmfAudioPump, bool isCaller, IntPtr userdata, _RTCSessionEventHandler observer);

    [UnmanagedFunctionPointer(JSEP_CALL,CharSet = CharSet.Ansi)]
    private delegate int PFN_CreateDataChannel (IntPtr iface, string channelId, string constraints);

    [UnmanagedFunctionPointer(JSEP_CALL,CharSet = CharSet.Ansi)]
    private delegate int PFN_CreateAnswer (IntPtr iface, string constraints);

    [UnmanagedFunctionPointer(JSEP_CALL,CharSet = CharSet.Ansi)]
    private delegate int PFN_CreateOffer (IntPtr iface, string constraints);

    [UnmanagedFunctionPointer(JSEP_CALL,CharSet = CharSet.Ansi)]
    private delegate int PFN_GetStats (IntPtr iface, string statsType, bool bDebug);

    [UnmanagedFunctionPointer(JSEP_CALL,CharSet = CharSet.Ansi)]
    private delegate int PFN_InsertDtmf (IntPtr iface, string tones, int duration, int inter_tone_gap);

    [UnmanagedFunctionPointer(JSEP_CALL,CharSet = CharSet.Ansi)]
    private delegate int PFN_PublishRemoteStream(IntPtr iface, string streamId, int renderOrCapturerBits, int videoTrackMask);

    [UnmanagedFunctionPointer(JSEP_CALL)]
    private delegate IntPtr PFN_ReleasePeerConnection (IntPtr iface);

    [UnmanagedFunctionPointer(JSEP_CALL,CharSet = CharSet.Ansi)]
    private delegate void PFN_RemoveLocalStream (IntPtr iface, string streamId);

    [UnmanagedFunctionPointer(JSEP_CALL,CharSet = CharSet.Ansi)]
    private delegate int PFN_SendMessage (IntPtr iface, string channelId, byte[] buffer, int length);

    [UnmanagedFunctionPointer(JSEP_CALL,CharSet = CharSet.Ansi)]
    private delegate int PFN_SetLocalDescription (IntPtr iface, string desc);

    [UnmanagedFunctionPointer(JSEP_CALL,CharSet = CharSet.Ansi)]
    private delegate int PFN_SetRemoteDescription (IntPtr iface, string desc);

    [UnmanagedFunctionPointer(JSEP_CALL)]
    private delegate int PFN_LogRtcEvent(IntPtr iface, byte[] filename, int max_size_mb);

    [UnmanagedFunctionPointer(JSEP_CALL)]
    private delegate  int PFN_SetBitrate(IntPtr iface, int current_bitrate_bps, int max_bitrate_bps, int min_bitrate_bps);

    [UnmanagedFunctionPointer(JSEP_CALL)]
    private delegate int PFN_DumpAudioProcessing(byte[] filename, int max_size_mb);

    [UnmanagedFunctionPointer(JSEP_CALL,CharSet = CharSet.Ansi)]
    private delegate IntPtr PFN_CreateWebSocket(string wsURL, string rtcWebSocketInit, IntPtr userdata, _RTCSocketEventHandler observer);

    [UnmanagedFunctionPointer(JSEP_CALL,CharSet = CharSet.Ansi)]
    private delegate IntPtr PFN_CreateWebSocketServer(string wsURL, string rtcWebSocketInit, IntPtr userdata, _RTCSocketEventHandler observer);

    [UnmanagedFunctionPointer(JSEP_CALL,CharSet = CharSet.Ansi)]
    private delegate IntPtr PFN_CreateIceSocket(string rtcConfiguration, string rtcIceParameters, bool isCaller, IntPtr userdata, _RTCSocketEventHandler observer);

    [UnmanagedFunctionPointer(JSEP_CALL)]
    private delegate IntPtr PFN_CloseSocket(IntPtr iface);

    [UnmanagedFunctionPointer(JSEP_CALL)]
    private delegate int PFN_SendSocket(IntPtr iface, byte[] buffer, int length, int sendFlags);

    [UnmanagedFunctionPointer(JSEP_CALL,CharSet = CharSet.Ansi)]
    private delegate int PFN_AddSocketIceCandidate(IntPtr iface, string candidate);

    [UnmanagedFunctionPointer(JSEP_CALL,CharSet = CharSet.Ansi)]
    private delegate int PFN_SetSocketIceParameters(IntPtr iface, string rtcIceParameters);

 #endregion
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    private const CallingConvention JSEP_CALL = CallingConvention.Cdecl;

    [DllImport("jsep.dll", EntryPoint = "JsepAPI", CallingConvention = JSEP_CALL)]
    private static extern IntPtr _JsepAPI(int apiLevel);

    [UnmanagedFunctionPointer(JSEP_CALL)]
    private delegate void _RTCSessionEventHandler(IntPtr user, int envetId, [MarshalAs(UnmanagedType.LPArray,SizeParamIndex=3)] byte[] json, int len);
    [UnmanagedFunctionPointer(JSEP_CALL)]
    private delegate void _RTCSocketEventHandler(IntPtr user, IntPtr iface, [MarshalAs(UnmanagedType.LPArray,SizeParamIndex=3)] byte[] utf8, int len, int eventId);
    private static Delegate[] OBSERVERS = new Delegate[256];
    private static Dictionary<IntPtr, RTCSocket> SOCKETS = new Dictionary<IntPtr, RTCSocket>();
    private static readonly _RTCSessionEventHandler RTCSessionCallback = (user, envetId, json, len) =>{
        RTCSessionObserver observer = (RTCSessionObserver)OBSERVERS[(int)user];
        if (observer != null)
            observer((RTCSessionEvent)envetId, Encoding.UTF8.GetString(json));
    };
       
    private static T JsepAPI<T>(int index) where T : class {
        Delegate api = Marshal.GetDelegateForFunctionPointer(Marshal.ReadIntPtr(JSEP_API, index * IntPtr.Size), typeof(T));
        if (api == null) throw new NotImplementedException();
        return api as T;
    }

    static JSEP()
    {
        try {
            JSEP_API = _JsepAPI(1);
            if (JSEP_API == IntPtr.Zero) return;
            int i=0;
            // media stream
            _AddLocalStream = JsepAPI<PFN_AddLocalStream>(i++);
            _RemoveLocalStream = JsepAPI<PFN_RemoveLocalStream>(i++);
            _PublishRemoteStream = JsepAPI<PFN_PublishRemoteStream>(i++);

            // data channel
            _CreateDataChannel = JsepAPI<PFN_CreateDataChannel>(i++);
            _CloseDataChannel = JsepAPI<PFN_CloseDataChannel>(i++);
            _SendMessage = JsepAPI<PFN_SendMessage>(i++);

            // peer
            _CreatePeerConnection = JsepAPI<PFN_CreatePeerConnection>(i++);
            _ReleasePeerConnection = JsepAPI<PFN_ReleasePeerConnection>(i++);

            // negotiation
            _CreateAnswer = JsepAPI<PFN_CreateAnswer>(i++);
            _CreateOffer = JsepAPI<PFN_CreateOffer>(i++);
            _AddIceCandidate = JsepAPI<PFN_AddIceCandidate>(i++); 
            _SetLocalDescription = JsepAPI<PFN_SetLocalDescription>(i++);
            _SetRemoteDescription = JsepAPI<PFN_SetRemoteDescription>(i++);


            // dynamic setting
            _InsertDtmf = JsepAPI<PFN_InsertDtmf>(i++);
            _SetBitrate = JsepAPI<PFN_SetBitrate>(i++);

            // debug
            _GetStats = JsepAPI<PFN_GetStats>(i++);
            _LogRtcEvent = JsepAPI<PFN_LogRtcEvent>(i++);
            _DumpAudioProcessing = JsepAPI<PFN_DumpAudioProcessing>(i++);

            // socket
            _CreateWebSocket = JsepAPI<PFN_CreateWebSocket>(i++);
            _CreateWebSocketServer = JsepAPI<PFN_CreateWebSocketServer>(i++);
            _CreateIceSocket = JsepAPI<PFN_CreateIceSocket>(i++);
            _CloseSocket = JsepAPI<PFN_CloseSocket>(i++);
            _SendSocket = JsepAPI<PFN_SendSocket>(i++);

            // ice socket
            _AddSocketIceCandidate = JsepAPI<PFN_AddSocketIceCandidate>(i++);
            _SetSocketIceParameters = JsepAPI<PFN_SetSocketIceParameters>(i++);

        }
        catch{
            JSEP_API = IntPtr.Zero;
        }
    }
}
