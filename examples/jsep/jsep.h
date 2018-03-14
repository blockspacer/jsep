/**
 * ģ�� WEBRTC 1.0 ��׼ʵ��1��1��ͨ������,
 *
 * �޶�:
 * - 2017.11.17 rik.gong    ���� IceSocket ����
 * - 2017.09.30 rik.gong    ʵ�� JESP �� WebSocket ���� 
 */

#ifndef __JSEP_H__
#define __JSEP_H__
#pragma once
#ifdef __cplusplus
#include <string>
#include <vector>
#endif
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <dlfcn.h>
#if defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
#define JSEP_IMPORT 1
#elif TARGET_OS_MAC
#include <libproc.h>
#endif
#endif
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/** �ӿں����汾��
 * JSEP_API�ṹ����,������´˰汾��
 *
 * @remarks
 *  �������´���, ����JSEP���Ƿ���سɹ�
 *      JsepAPI(JSEP_API_LEVEL) ? "ok" : "failed"
 *
 *  ��������
 *   - JSEP_LOG_TO_DEBUG �ɸı���־����,��ѡֵ "INFO", "WARN", "ERROR"
 *   - JSEP_LOG_DIR_PATH �ɽ���־д��Ŀ¼
 */
#define JSEP_API_LEVEL      1

typedef int RTCBoolean;
typedef struct RTCPeerConnection RTCPeerConnection;
typedef struct RTCSocket RTCSocket;
///////////////////////////////////////////////////////////////////////////////
/**
 * @defgroup JSON ��ʽ�����ò�������
 * @{
 */

/**
 * ͨ�����ò�������.
 * ����
 * {'iceServers': [{'urls': [ url1, url2 ]}]}
 *
 * ֧�ֵ����ò���
 * - bundlePolicy:'balanced'
 * - rtcpMuxPolicy:'require' | 'negotiate'
 * - iceTransportPolicy:'all'
 * - STRUN ��������ַ
 *  iceServers: [{
 *          urls: [ 'url' ],
 *          username:'',
 *          credential:'',
 *  }]
 * - ���Ʋ���
 *  constraints:{
 *      minPort: 0
 *      maxPort: 0
 *      googIPv6: true
 *      googDscp: true
 *      DtlsSrtpKeyAgreement: true
 *      RtpDataChannels: false
 *      googSuspendBelowMinBitrate: true
 *      googNumUnsignalledRecvStreams: 20
 *      googScreencastMinBitrate:<int>
 *      googHighStartBitrate:<int>
 *      googHighBitrate:true
 *      googVeryHighBitrate:true
 *      googCombinedAudioVideoBwe:true
 *  }
 * - ��ѡ����
 *   options: {
 *      networkIgnoreMask: 16 //��������, ��ѡֵ: ETHERNET=1, WIFI=2, CELLULAR=4, VPN=8, LOOPBACK=16
 *      disableEncryption: false //�ر� SRTP
 *   }
 */
typedef const char* RTCConfiguration;

/** ICE ��ѡ��ַ */
typedef const char* RTCIceCandidate;

/**
 * Ice���ӵ����ò���
 *
 * Ŀǰ֧��
 * - username �� usernameFragment �� ufrag
 * - password
 * - iceLite �� renomination
 */
typedef const char* RTCIceParameters;

/**
 * ����Ƶý������Ʋ���
 *
 * Ŀǰ֧��
 * - ��Ƶ
 *    video: {
 *      //��ZMF�ԽӵĲ���
 *      zmfCapture:Id0      //ZMF�ɼ�ԴID
 *      zmfRender:Id0       //ZMF��ȾԴID,ת��Ϊ��Ƶ�ɼ�Դ
 *
 *      //������ֵ����
 *      maxWidth,minWidth,  //�������
 *      maxHeight,minHeight,//�߶�����
 *      maxFrameRate,minFrameRate,//֡��������
 *      maxAspectRatio,minAspectRatio,//��ȱ�����
 *      googNoiseReduction,//�Ƿ���
 *    }
 * - ��Ƶ
 *    audio: {
 *      DTMF:false          //�ر�DTMF,�ɽ�ʡЩ�ڴ�.
 *
 *      //���ò�������
 *      echoCancellation,googEchoCancellation,googEchoCancellation2, googDAEchoCancellation,
 *      googAutoGainControl,googAutoGainControl2,
 *      googNoiseSuppression,googNoiseSuppression2,
 *      intelligibilityEnhancer,
 *      levelControl,
 *      googHighpassFilter,
 *      googTypingNoiseDetection,
 *      googAudioMirroring,
 *      levelControlInitialPeakLevelDBFS,
 *      googAudioNetworkAdaptorConfig
 *    }
 *  ��
 *    audio: false          //ǿ�ƹر���Ƶ
 */
typedef const char* MediaStreamConstraints;

typedef const char* RTCAnswerOptions;

typedef const char* RTCSessionDescription;


/**
 * Э�̹��̵� Offer ��������
 *
 * Ŀǰ֧��
 *  - OfferToReceiveAudio: true
 *  - OfferToReceiveVideo: true
 *  - VoiceActivityDetection: true
 *  - IceRestart: false
 *  - googUseRtpMUX: true
 */
typedef const char* RTCOfferOptions;

/**
 * ��������ͨ�� DataChannel �����ò���
 *
 * Ŀǰ֧��
 * - ordered:true,       //�Ƿ�֤����,Ĭ��true
 * - maxPacketLifeTime:0,//������ʱ,�����ط�.Ĭ��-1,ʼ���ط�
 * - maxRetransmits:0,   //��������,�����ط�,Ĭ��-1,,ʼ���ط�
 * - protocol:'',        //�Զ�����ϲ�Ӧ��Э����,Ĭ�Ͽ�
 * - negotiated:false,   //�Ƿ����ϲ�Ӧ�ø���Э�̽�������,
 *                         ��������OnChannelOpen�¼�.Ĭ��false���ڲ��Զ����
 */
typedef const char* RTCDataChannelInit;

/**
 * WebSocket ���ò���
 *
 * �ͻ�������Ŀǰ֧��
 * - origin:''    //��ѡ����Դ��ʶ
 * - protocols:'' //��ѡ��Ӧ�ò�Э��,���Э����','�ֿ�
 * - ca_certs:'cert.pem' //CA֤�����ڵ�PEM�ļ���Ŀ¼, ������֤�Է�֤��, '*' ��ʾʹ��ϵͳ֤��
 * - hostname:''         //�ͻ���������������,������֤�Է�֤���е�����
 * - certfile:'mypub.pem'  //����֤��(��Կ)
 * - keyfile:'mykey.pem'   //����˽Կ
 * - cert_reqs:0  //��ϸ�μ� SSL_VERIFY_*
 * - ignore_bad_cert:false //���ԶԶ�֤��(��Կ)����֤����,�����ڵ���
 * - timeout:2000 //��������ֵĳ�ʱʱ��
 * - backlog:100  //�������������󳤶�
 * - ipv6:false   //�����������ַΪ''��*ʱ,ָ��Ϊin6addr_any, Ĭ����INADDR_ANY
 */
typedef const char* RTCWebSocketInit;

/**
 * ��ϸ��ͳ������
 * ����ͳ�����Ͷ�Ӧ��ͬ�� JSON ��ʽ
 *
 * Ŀǰ֧������ͳ������
 * - googLibjingleSession  ȫ�ֻỰ
 * - transport             �����
 * - VideoBwe              ��Ƶ�������
 * - remoteSsrc            �Զ�RTP��
 * - ssrc                  RTP��
 *   ��Ƶ: {bytesSent:0,codecImplementationName:'',framesEncoded:0}
 *   ��Ƶ: {}
 *
 * - googTrack             ý����
 *   {googTrackId:'video0'}
 *
 * - localcandidate        ����ICE
 *   {candidateType:'host',ipAddress:'192.168.0.240',networkType:'unknown', portNumber:'53217',priority:1518018303,transport:'tcp'}
 *
 * - remotecandidate       �Զ�ICE
 *   {candidateType:'host',ipAddress:'192.168.0.240',portNumber:'53217',priority:1518018303,transport:'tcp'}
 *
 * - googComponent
 *
 * - googCandidatePair
 *
 * - googCertificate
 *
 * - datachannel           ����ͨ��
 *   {datachannelid:1,label:'',protocol:'',state:'open'}
 */
typedef const char* RTCStats;
/** @} */
//////////////////////////////////////////////////////////////////////////////
/**
 * @defgroup RTCSessionEvent�¼�Я��JSON�������ֶ���
 * @{
 */
/** �¼�����, ��������:enum RTCSessionEvent */
#define JespEvent                           "JespEvent"

/** SDP����, ��������:UTF8�ַ���, ��ѡֵ
 * - "offer" ��������
 * - "answer" Ӧ������
 */
#define JsepSdpType                         "JsepSdpType"

/** SDPλ��, ��������:UTF8�ַ���, ��ѡֵ
 * - "local"  ���ؽ��ղ�
 * - "remote" ��Ӧ���Ͳ�
 */
#define JsepSdpLocation                     "JsepSdpLocation"

/** SDP����,��������: JSON�� @seee RTCSessionDescription */
#define JsepSdp                             "JsepSdp"

/** ԭ��, ��������: UTF8�ַ��� */
#define JsepReason                          "JsepReason"

/** ICE��ַ, ��������: JSON�� @see RTCIceCandidate */
#define JsepIceCandidate                    "JsepIceCandidate"

/** ICE����״̬, ��������: UTF8�ַ���, ��ѡֵ
 * - "new" ��ʼ״̬,
 * - "checking" ̽�����״̬
 * - "connected" �ɹ�����
 * - "completed" ̽�����
 * - "disconnected" �Ͽ�����
 * - "failed" ���ӹ�����ʧ��,���ɻָ�
 * - "closed", �ر�����
 * ֻ����connected��completed״̬��,�ſɽ���ͨ�Ź���.
 */
#define JsepIceConnectionState              "JsepIceConnectionState"

/** Э��״̬, ��������: UTF8�ַ���, ��ѡֵ
 * - "stable", û�н���Э�̵��ȶ�״̬.
 * - "have-local-offer", �ѵ�SetLocalDescription(offer),������ղ�����SetRemoteDescription(answer)
 * - "have-remote-offer",�ѵ�SetRemoteDescription(offer),���봴����Ӧ��SetLocalDescription(answer)
 * - "have-remote-pranswer",�ѵ�SetRemoteDescription(answer),û��SetLocalDescription(offer)״̬
 * - "have-local-pranswer",�ѵ�SetLocalDescription(answer),û��SetRemoteDescription(offer)״̬
 * - "closed",�����ѹرյ�״̬.
 */
#define JsepSignalingState                  "JsepSignalingState"

/** ý����ID, ��������: UTF8�ַ��� */
#define JsepStreamId                        "JsepStreamId"

/** ý��������Ƶ�������, ��������: int */
#define JsepAudioTrackCount                 "JsepAudioTrackCount"

/** ý��������Ƶ�������, ��������: int */
#define JsepVideoTrackCount                 "JsepVideoTrackCount"

/** ����ͨ��ID, ��������: UTF8�ַ��� */
#define JsepChannelId                       "JsepChannelId"

/** ����ͨ������, ��������: JSON�� @see RTCDataChannelInit */
#define JsepChannelConfig                   "JsepChannelConfig"

/** ��Ϣ����, ��������: UTF8�ַ��� */
#define JsepMessage                         "JsepMessage"

/** DTMF �ѷ�������, ��������: UTF8�ַ���
 * ��Ϊ""��nullptr���ʾ�ѷ������
 */
#define JsepTone                            "JsepTone"

/** ͳ������, ��������: UTF8�ַ���, ��ѡֵ
 * - googLibjingleSession  ȫ�ֻỰ
 * - transport             �����
 * - VideoBwe              ��Ƶ�������
 * - remoteSsrc            �Զ�RTP��
 * - ssrc                  RTP��
 * - googTrack             ý����
 * - localcandidate        ����ICE
 * - remotecandidate       �Զ�ICE
 * - googComponent
 * - googCandidatePair
 * - googCertificate
 * - datachannel           ����ͨ��
 */
#define JsepStatsType                       "JsepStatsType"

/** ͳ��ID, ��������: UTF8�ַ��� */
#define JsepStatsId                         "JsepStatsId"

/** ͳ��ʱ���, ��������: double */
#define JsepStatsTimestamp                  "JsepStatsTimestamp"

/** ͳ��ֵ, ��������: JSON�� @see RTCStats */
#define JsepStats                           "JsepStats"

/** @} */
///////////////////////////////////////////////////////////////////////////////
/**
 * ͨ���еĴ���ֵ
 */
enum RTCSessionError {
    /** ��Ч����,δ֪���� */
    RTCSessionError_InvalidOperation    = -1,
    /** �Ƿ��Ĳ���ʹ�� */
    RTCSessionError_InvalidArgument     = -2,
    /** û����Ƶ�� */
    RTCSessionError_MissingVideoTrack   = -3,
    /** �汾��ƥ�� */
    RTCSessionError_MismatcheaVersion   = -4,
};
///////////////////////////////////////////////////////////////////////////////
/**
 * RTCSocket �¼�
 * �ڲ�ʹ��RTCSocketObserver, ����ʹ��ͳһ�Ļص���ʱ,
 * void (JSEP_CDECL_CALL *observer)(RTCSocketObserver*, RTCSocket*, const char* data, int, enum RTCSocketEvent)
 * ͨ����ͬ���¼���ʶ���� data ��ʵ�ʺ���
 */
enum RTCSocketEvent {
    /** �յ���Ϣ. ��Ӧ RTCSocketObserver::OnSocketMessage */
    RTCSocketEvent_Message = 0,
    /** ״̬�ı�,������Ϊ��״̬. ��Ӧ RTCSocketObserver::OnSocketStateChange */
    RTCSocketEvent_StateChange,
    /** ��ȡ�µı��غ�ѡ��ַ, ������Ϊ��ѡ��ַ. ��Ӧ RTCSocketObserver::OnSocketIceCandidate */
    RTCSocketEvent_IceCandidate,
};
///////////////////////////////////////////////////////////////////////////////
/**
 * GetStats�ӿڵĲ���
 * ����ϸ�ֻ����ͳ�ƽ��
 */
enum RTCStatsFlag {
    /** ��ϸ��, ���Լ����ͳ�� */
    RTCStatsFlag_Debug = 1,
    /** ������Ƶ��ص�ͳ�� */
    RTCStatsFlag_Audio = 2,
    /** ������Ƶ��Ӧ��ͳ�� */
    RTCStatsFlag_Video = 4,
};
///////////////////////////////////////////////////////////////////////////////
/**
 * @defgroup ͨ���еĻ����¼�, Я����JSON��ʽ����
 *  ��ʹ��RTCSessionObserver, ����ʹ��ͳһ�Ļص�����
 *  void (JSEP_CDECL_CALL *observer)(RTCSessionObserver*, enum RTCSessionEvent event, const char* json, int length)
 *  ͨ����ͬ���¼���ʶ,���� json �Ķ�Ӧʵ�ֺ���
 * @{
 */
enum RTCSessionEvent {
    /**
     * ��Ҫ��������Э��
     * ��Ӧ RTCSessionObserver::OnRenegotiationNeeded
     *
     * @remarks
     *  ͨ�����JSEP_CreateOffer(),���ظ�Э������
     */
    RTCSessionEvent_RenegotiationNeeded = 1,

    /**
     * ���� SDP �ɹ�
     * ��Ӧ RTCSessionObserver::OnCreateDescriptionSuccess
     *  {JsepSdpType:%s,JsepSdp:{...}}
     *
     * @remarks
     *  Э��ʱ,��Ҫ����JSEP_SetLocalDescription()���ø�SDP
     */ 
    RTCSessionEvent_CreateDescriptionSuccess,

    /**
     * ���� SDP ʧ��
     * ��Ӧ RTCSessionObserver::OnCreateDescriptionFailure
     *  {JsepSdpType:%s,JsepReason:%s}
     *
     */ 
    RTCSessionEvent_CreateDescriptionFailure,

    /**
     * ���� SDP �ɹ�
     * ��Ӧ RTCSessionObserver::OnSetDescriptionSuccess
     *  {JsepSdpType:%s,JsepSdpLocation:%s,JsepSdp:{...}}
     *
     * @remarks
     *  ����remote offer, ����Ҫ������Ӧ�� answer SDP,�������Զ�
     */
    RTCSessionEvent_SetDescriptionSuccess,

    /**
     * ���� SDP ʧ��
     * ��Ӧ RTCSessionObserver::OnSetDescriptionFailure
     *  {JsepSdpType:%s,JsepSdpLocation:%s, JsepReason:%s}
     */
    RTCSessionEvent_SetDescriptionFailure,

    /**
     * �µ�ICE��ѡ��ַ
     * ��Ӧ RTCSessionObserver::OnIceCandidate
     *  {JsepIceCandidate:{...}}
     *
     * @remarks
     *  Э��ʱ,����ICE��ַͨ������֪ͨ�Զ�
     */
    RTCSessionEvent_IceCandidate,

    /**
     * ICE����״̬�ı�
     * ��Ӧ RTCSessionObserver::OnIceConnectionStateChange
     * {JsepIceConnectionState:%s}
     */
    RTCSessionEvent_IceConnectionStateChange,

    /**
     * Э��״̬�ı�
     * ��Ӧ RTCSessionObserver::OnSignalingChange
     * {JsepSignalingState:%s}
     */
    RTCSessionEvent_SignalingChange,

    /**
     * ����ӶԶ�ý����
     * ��Ӧ RTCSessionObserver::OnAddRemoteStream
     *  {JsepStreamId:%s,JsepAudioTrackCount:%d,JsepVideoTrackCount:%d}
     *
     * @remarks
     *  ��Ҫ����JSEP_PublishRemoteStream(),��������,�Ա���Zmf�Խ�
     */
    RTCSessionEvent_AddRemoteStream,

    /**
     * ���Ƴ��Զ�ý����
     * ��Ӧ RTCSessionObserver::OnRemoveRemoteStream
     *  {JsepStreamId:%s}
     */
    RTCSessionEvent_RemoveRemoteStream,

    /**
     * DTMF �����ѷ���
     * ��Ӧ RTCSessionObserver::OnToneChange
     *  {JsepTone:%s}
     */
    RTCSessionEvent_ToneChange,

    /**
     * �յ�ͳ�Ʊ���
     * ��Ӧ RTCSessionObserver::OnStatsReport
     *  {JsepStatsType:%s,JsepStatsId:%s,JsepStatsTimestamp:%f,JsepStats:{...}}
     */
    RTCSessionEvent_StatsReport,

    /**
     * �Ѵ�����ͨ��,���շ���Ϣ
     * ��Ӧ RTCSessionObserver::OnDataChannelOpen
     *  {JsepChannelId:%s,JsepChannelConfig:{...}}
     */
    RTCSessionEvent_DataChannelOpen,

    /**
     * �յ�����
     * ��Ӧ RTCSessionObserver::OnDataChannelMessage
     *  {JsepChannelId:%s,JsepMessage:%s}
     */
    RTCSessionEvent_DataChannelMessage,

    /**
     * �ѹر�����ͨ��,������ʹ��JSEP_SendMessage����
     * ��Ӧ RTCSessionObserver::OnDataChannelClose
     *  {JsepChannelId:%s,JsepReason:%s}
     */
    RTCSessionEvent_DataChannelClose,
};

#ifdef __cplusplus
/** C++��Ա�������÷�ʽ */
#if __GNUC__
#define JSEP_THIS_CALL
#else
#define JSEP_THIS_CALL __thiscall
#endif
/**
 * �Ự����������, ��Windowsƽ̨�ⶼ���ڶ����߳��лص�.
 * C++��ͨ���̳иýӿ�,ʹ�ûص���ʽ������JSON�Ķ�������.
 */
class RTCSessionObserver {
public:
    /**
     * ����SDP�ɹ�
     * @see RTCSessionEvent_CreateDescription
     *
     * @param[in] type SDP����,��ѡֵΪ"offer","answer"
     * @param[in] rtcSessionDescription SDPʵ��, @see RTCSessionDescription
     *
     * @remarks
     *  Э��ʱ,��Ҫ����JSEP_SetLocalDescription()���ø�SDP
     */
    virtual void JSEP_THIS_CALL OnCreateDescriptionSuccess(const char* type, RTCSessionDescription rtcSessionDescription) = 0;

    /**
     * ����SDPʧ��
     * @see RTCSessionEvent_CreateDescription
     *
     * @param[in] type SDP����,��ѡֵΪ"offer","answer"
     * @param[in] error ����ԭ��
     */
    virtual void JSEP_THIS_CALL OnCreateDescriptionFailure(const char* type, const char* error) = 0;

    /**
     * ����SDP�ɹ�
     * @see RTCSessionEvent_SetDescription
     *
     * @param[in] type SDP����,��ѡֵΪ"offer","answer"
     * @param[in] locate λ��, ��ѡֵΪ"local","remote"
     * @param[in] rtcSessionDescription  SDP�ַ���,@see RTCSessionDescription
     *
     * @remarks
     *  Э��ʱ,����SDP֪ͨ�Զ�,
     *  ����"remote","offer",����Ҫ������Ӧ��"answer",�������Զ�
     */
    virtual void JSEP_THIS_CALL OnSetDescriptionSuccess(const char* type, const char* locate, RTCSessionDescription rtcSessionDescription) = 0;

    /**
     * ����SDPʧ��
     * @see RTCSessionEvent_SetDescription
     *
     * @param[in] type SDP����,��ѡֵΪ"offer","answer"
     * @param[in] locate λ��, ��ѡֵΪ"local","remote"
     * @param[in] error ����ԭ��
     */
    virtual void JSEP_THIS_CALL OnSetDescriptionFailure(const char* type, const char* locate, const char* error) = 0;

    /**
     * ����ICE��ַ
     *
     * @param[in] rtcIceCandidate ���ر�ѡ��ַ, @see RTCIceCandidate
     *
     * @see RTCSessionEvent_IceCandidate
     *
     * @remarks
     *  Э��ʱ,����ICE��ַͨ������֪ͨ�Զ�
     */
    virtual void JSEP_THIS_CALL OnIceCandidate(RTCIceCandidate rtcIceCandidate) = 0;

    /**
     * ����ICE����״̬
     * @see RTCSessionEvent_IceConnectionStateChange
     *
     * @param[in] iceState ״̬, @see JsepIceConnectionState
     *
     */
    virtual void JSEP_THIS_CALL OnIceConnectionStateChange(const char* iceState) = 0;

    /**
     * ����Э��״̬
     * @see RTCSessionEvent_SignalingChange
     *
     * @param[in] signalingState ״̬, @see JsepSignalingState
     */
    virtual void JSEP_THIS_CALL OnSignalingChange(const char* signalingState) = 0;

public:
    /**
     * ����ӶԶ�ý����
     * @see RTCSessionEvent_AddRemoteStream
     *
     * @param[in] streamId  ��Id
     * @param[in] audioTrackCount ��Ƶ����
     * @param[in] videoTrackCount ��Ƶ����
     */
    virtual void JSEP_THIS_CALL OnAddRemoteStream(const char* streamId, int audioTrackCount, int videoTrackCount) = 0;

    /**
     * ���Ƴ��Զ�ý����
     * @see RTCSessionEvent_RemoveRemoteStream
     */
    virtual void JSEP_THIS_CALL OnRemoveRemoteStream(const char* streamId) = 0;

    /**
     * ��Ҫ��������Э��
     * @see RTCSessionEvent_RenegotiationNeeded
     */
    virtual void JSEP_THIS_CALL OnRenegotiationNeeded(void) = 0;

public:
    /**
     * DTMF �����ѷ���
     * @see RTCSessionEvent_ToneChange
     *
     * @param[in] tone �ѷ�������.��Ϊ""����ʾ�ѷ������
     */
    virtual void JSEP_THIS_CALL OnToneChange(const char* tone) = 0;

    /**
     * ͳ�Ʊ���
     * @see RTCSessionEvent_StatsReport
     *
     * @param[in] statsType ͳ�����,@see JsepStatsType
     * @param[in] statsId ͳ��ID
     * @param[in] stats ��ϸ����,JSON��ʽ, @see RTCStats
     * @param[in] timestamp ʱ���
     */
    virtual void JSEP_THIS_CALL OnStatsReport(const char* statsType, const char* statsId, RTCStats stats, const double timestamp) = 0;

public:
    /**
     * �Ѵ�����ͨ��,���շ���Ϣ
     * @see RTCSessionEvent_DataChannelOpen
     *
     * @param[in] channelId ͨ��ID
     * @param[in] config ͨ����Ӧ��JSON����,@see RTCDataChannelInit
     */
    virtual void JSEP_THIS_CALL OnDataChannelOpen(const char* channelId, RTCDataChannelInit config) = 0;

    /**
     * �յ��Զ�����
     * @see RTCSessionEvent_DataChannelMessage
     *
     * @param[in] channelId ͨ��ID
     * @param[in] buffer ��Ϣ
     */
    virtual void JSEP_THIS_CALL OnDataChannelMessage(const char* channelId, const char* buffer, int length) = 0;

    /**
     * �ѹر�����ͨ��
     * @see RTCSessionEvent_DataChannelClose
     *
     * @param[in] channelId ͨ��ID
     * @param[in] reason �ر�ԭ��
     */
    virtual void JSEP_THIS_CALL OnDataChannelClose(const char* channelId, const char* reason) = 0;

protected:
    virtual ~RTCSessionObserver(){}
};

/**
 * RTCSocket�¼�������
 */
class RTCSocketObserver {
public:
    /** ״̬�ı�
     * 
     * @param[in] state ״ֵ̬
     *
     * @remarks
     *  ״̬��ѡֵ
     *  new
     *  checking
     *  completed
     *  connected[: local=candidate; remote=candidate]
     *  open[: protocol]
     *  closed
     *  failed
     */
    virtual void JSEP_THIS_CALL OnSocketStateChange(RTCSocket* rtcsocket, const char* state) = 0;

    /**
     * ����ICE��ַ
     *
     * @param[in] candidate ��ַʵ��
     *
     * @remarks
     *  Э��ʱ,����ICE��ַͨ������֪ͨ�Զ�
     */
    virtual void JSEP_THIS_CALL OnSocketIceCandidate(RTCSocket* rtcsocket, const char* candidate) = 0;

    /** �յ�����������
     *
     * @param[in] buffer ��Ϣ
     */
    virtual void JSEP_THIS_CALL OnSocketMessage(RTCSocket* rtcsocket, const char* buffer, int length) = 0;

protected:
    virtual ~RTCSocketObserver(){}
};

#else
typedef struct RTCSessionObserver RTCSessionObserver;
typedef struct RTCSocketObserver  RTCSocketObserver;
#endif

#ifdef __OBJC__
/**
 * ����iOSϵͳ,��������ʱ,
 * ����NSNotification�ķ�ʽ�㲥.
 * �¼������� JsmReplyNotification,
 * NSNotification.userInfo Ϊ�¼�NSDictionary,
 * �����¼�RTCSessionEventֵ���ֵ�ؼ���Ϊ JespEvent
 */
@class NSString;
extern NSString * const JsepNotification;
#endif

/** @} */
///////////////////////////////////////////////////////////////////////////////
/**
 * @defgroup Э������
 *
 *  ��������:
 *      - JSEP_AddLocalStream()         ���ý����
 *      - JSEP_CreateOffer()            ����offer SDP
 *      - JSEP_SetLocalDescription()    ����offer SDP
 *      - �� offer SDP ֪ͨ�Զ�
 *
 *  �Զ�����:
 *      - �յ�offer SDP
 *      - JSEP_AddLocalStream()         ���ý����
 *      - JSEP_SetRemoteDescription()   ����offer SDP
 *      - JSEP_CreateAnswer()           ����answer SDP
 *      - JSEP_SetLocalDescription()    ����answer SDP
 *      - �� answer SDP �����Զ�
 *
 *  ��������
 *      - �յ�answer SDP
 *      - JSEP_SetRemoteDescription()   ����answer SDP
 * @{
 */

/**
 * ��ӱ���ý����
 *
 * @param[in] iface P2Pͨ��ʵ��
 * @param[in] streamId ý����ID
 * @param[in,out] bAudio �Ƿ���Ƶ
 * @param[in,out] bVideo �Ƿ���Ƶ
 * @param[in] constraints ý���������Ʋ���,@see MediaStreamConstraints
 *
 * @return �ɹ�����0, bAudio, bVideo�ֱ�ָʾ�Ƿ�����Ƶ
 *
 * @@remarks
 *  ����ý����ͨ����Э��ǰ���,�Զ�ý�������Զ���ӵ�.
 *  streamId Ҫ��֤Ψһ��,ͨ�����˻�ID.
 */
#define JSEP_AddLocalStream(iface,streamId,bAudio, bVideo, constraints) \
    JsepAPI(JSEP_API_LEVEL)->AddLocalStream(iface, streamId, bAudio, bVideo, constraints)

/** 
 * �Ƴ�����ý����
 *
 * @param[in] iface P2Pͨ��ʵ��
 * @param[in] streamId ý����ID,��""��nullptr���Ƴ����б�����
 */
#define JSEP_RemoveLocalStream(iface, streamId) \
    JsepAPI(JSEP_API_LEVEL)->RemoveLocalStream(iface, streamId)

/**
 * ��ý����������ZMF��
 *
 * @param[in] iface P2Pͨ��ʵ��
 * @param[in] renderOrCapturerBits ��λ����Ƶ�����Ϊ0-ZMF��Ⱦ,1-ZMF��ͷ
 * @param[in] videoTrackMask  �ϸ�����renderOrCapturerBits�е���Чλ����
 *
 * @return �ɹ�����0
 */
#define JSEP_PublishRemoteStream(iface, streamId, renderOrCapturerBits, videoTrackMask) \
    JsepAPI(JSEP_API_LEVEL)->PublishRemoteStream(iface, streamId, renderOrCapturerBits, videoTrackMask)

/**
 * ��ӶԶ˵�ICE��ѡ��ַ
 *
 * @param[in] iface P2Pͨ��ʵ��
 * @param[in] rtcIceCandidate �Զ˵�ַ, @see RTCIceCandidate
 *            ͨ���Զ��� RTCSessionEvent_IceCandidate �¼���JsepIceCandidate�ֶλ��
 *
 * @return �ɹ�����0
 */
#define JSEP_AddIceCandidate(iface, rtcIceCandidate) \
    JsepAPI(JSEP_API_LEVEL)->AddIceCandidate(iface, rtcIceCandidate)

/**
 * ����offer SDP
 *
 * @param[in] iface P2Pͨ��ʵ��
 * @param[in] rtcOfferOptions ��ѡ��SDP�����Ʋ���, @see RTCOfferOptions
 *
 * @return ��Ч������������-1,��֮�첽����,�����RTCSessionEvent_CreateDescription �¼�����.
 */
#define JSEP_CreateOffer(iface, rtcOfferOptions) \
    JsepAPI(JSEP_API_LEVEL)->CreateOffer(iface, rtcOfferOptions)

/**
 * ���ñ���(����)��SDP
 *
 * @param[in] iface P2Pͨ��ʵ��
 * @param[in] rtcSessionDescription SDP��JSON��, @see RTCSessionDescription
 *            ͨ�������� RTCSessionEvent_CreateDescription �¼���JsepSdp���
 *
 * @return ��Ч������������-1,��֮�첽����,����� RTCSessionEvent_EventSetDescription �¼�����.
 */
#define JSEP_SetLocalDescription(iface, rtcSessionDescription) \
    JsepAPI(JSEP_API_LEVEL)->SetLocalDescription(iface, rtcSessionDescription)

/**
 * ���öԶ�(����)��SDP
 *
 * @param[in] iface P2Pͨ��ʵ��
 * @param[in] rtcSessionDescription SDP��JSON��,@see RTCSessionDescription
 *            ͨ���Զ��� RTCSessionEvent_CreateDescription �¼���JsepSdp���
 *
 * @return ��Ч������������-1,��֮�첽����,�����RTCSessionEvent_SetDescription �¼�����.
 */
#define JSEP_SetRemoteDescription(iface, rtcSessionDescription) \
    JsepAPI(JSEP_API_LEVEL)->SetRemoteDescription(iface, rtcSessionDescription)

/**
 * ���� answer SDP
 *
 * @param[in] iface P2Pͨ��ʵ��
 * @param[in] rtcAnswerOptions SDP�����Ʋ���,@see JSEP_CreateOffer
 *
 * @return ��Ч������������-1,��֮�첽����,����� RTCSessionEvent_CreateDescription �¼�����.
 */
#define JSEP_CreateAnswer(iface,rtcAnswerOptions) \
    JsepAPI(JSEP_API_LEVEL)->CreateAnswer(iface, rtcAnswerOptions)

/** @} */
///////////////////////////////////////////////////////////////////////////////
/**
 * @defgroup ͨ������ͨ��
 *
 *  - ��������ͨ��,����Ҫ���½���Э�̹���
 *  - �Զ˽��Զ�������Ӧ������ͨ��.
 *  - ���غͶԶ˶����յ� RTCSessionEvent_DataChannelOpen �¼�,֮�󷽿�ʹ�ø�ͨ��.
 *  - �� RTCSessionEvent_DataChannelClose �¼���,ͨ����ʧЧ
 *
 * @{
 */

/**
 * ��̬��������ͨ��
 *
 * @param[in] iface P2Pͨ��ʵ��
 * @param[in] channelId ͨ��ID
 * @param[in] rtcDataChannelInit ���ò���, @see RtcDataChannelInit
 *
 * @return �ɹ�����0
 */
#define JSEP_CreateDataChannel(iface, channelId, rtcDataChannelInit) \
    JsepAPI(JSEP_API_LEVEL)->CreateDataChannel(iface, channelId, rtcDataChannelInit)

/**
 * �ر�����ͨ��
 *
 * @param[in] iface P2Pͨ��ʵ��
 * @param[in] channelId ͨ��ID
 */
#define JSEP_CloseDataChannel(iface, channelId) \
    JsepAPI(JSEP_API_LEVEL)->CloseDataChannel(iface, channelId)

/**
 * �����Զ�������
 *
 * @param[in] iface P2Pͨ��ʵ��
 * @param[in] channelId ͨ��ID
 * @param[in] message ��Ϣ
 * @param[in] length  ��Ϣ����, ������,���ʾmessage�ַ�������
 *
 * @return �ɹ�����0
 *
 * @remarks
 *  ������ RTCSessionEvent_DataChannelOpen �¼���,ͨ�������շ�����
 */
#define JSEP_SendMessage(iface, channelId, message, length) \
    JsepAPI(JSEP_API_LEVEL)->SendMessage(iface, channelId, message, length)

/** @} */
///////////////////////////////////////////////////////////////////////////////
/**
 * @defgroup ����ʵ��
 * @{
 */

/**
 * �ڲ�'ȫ����Ƶ����'ID,���������/����豸.
 * �ⲿ��֮�Խ���Ƶʱ,��ʹ�ø�ID
 */
#define JSEP_AUDIO_PUMP     " jsepAudioPump"

/**
 * ����P2Pͨ��ʵ��
 *
 * @param[in] rtcConfiguration  RTC���ò���, @see RTCConfiguration
 * @param[in] zmfAudioPump �Ƿ�ʹ�����õ�'ȫ����Ƶ��',�����ⲿ��Ƶ���Խ�, @see JSEP_AUDIO_PUMP
 * @param[in] isCaller �Ƿ�Ϊ����
 * @param[in] observer ����������
 * @param[in] callback ͳһ��JSON�¼��ص�����,��Windowsƽ̨�ⶼ�������߳��лص�
 *
 * @return �ɹ�����P2Pͨ��ʵ��
 *
 * @remarks
 *  �ͷ���ʹ��JSEP_Release()
 *  callback��Чʱ,��observer��Ϊ�����,������ʹ��,����ֱ�ӻص�observer.
 */
#define JSEP_RTCPeerConnection(rtcConfiguration, zmfAudioPump, isCaller, observer, callback)\
    JsepAPI(JSEP_API_LEVEL)->CreatePeerConnection(rtcConfiguration, zmfAudioPump, isCaller, observer, callback)

/**
 * �رղ��ͷ�P2Pͨ��ʵ��
 *
 * @param[in] iface P2Pͨ��ʵ��.ִ�к�,�ö���ʧЧ
 *
 * @return ���ش���ʱ���õ� observer
 */
#define JSEP_Release(iface) \
    JsepAPI(JSEP_API_LEVEL)->ReleasePeerConnection(iface)

/** @} */
///////////////////////////////////////////////////////////////////////////////
/**
 * @defgroup ���⹦��
 * @{
 */

/**
 * ����DTMF
 *
 * @param[in] iface P2Pͨ��ʵ��
 * @param[in] tones DMTF������,0-9,A-D��a-d,#,*. �����޷�ʶ����ַ�.
 * @param[in] duration_ms ÿ�������ĳ�������MS,���ܳ���6000��С��70
 * @param[in] inter_tone_gap �������,����Ϊ50ms,��Ӧ�����ܶ�
 *
 * @return �ɹ�����0,������ RTCSessionEvent_ToneChange �¼�.
 *
 * @remarks
 *  ���л�����DTMF ����.
 *  ','��ʾ�ӳ�2�봦����һ���ַ�
 *  ������ʱ,�ϴ���������,��֮ǰ�����񽫱�ȡ��
 */
#define JSEP_InsertDtmf(iface, tones, duration_ms, inter_tone_gap) \
    JsepAPI(JSEP_API_LEVEL)->InsertDtmf(iface, tones, duration_ms, inter_tone_gap)

/**
 * ��ȡͳ��
 *
 * @param[in] iface P2Pͨ��ʵ��
 * @param[in] statsType ͳ�����, @see JsepStatsType
 * @param[in] statsFlags  ͳ�Ʊ�ʶ, @see RTCStatsFlag
 *
 * @remarks
 *  statsType ���������
 *  - ''��0��ʾ��ȡ����ͳ��
 *  - ���� RTCStatsFlag_Audio �� RTCStatsFlag_Video ʱ, statsType ��ΪstreamId
 *  - ָ�����, �����','�ֿ�
 *
 * @return �ɹ�����0, ������ RTCSessionEvent_StatsReport �¼�.
 */
#define JSEP_GetStats(iface, statsType, statsFlags) \
    JsepAPI(JSEP_API_LEVEL)->GetStats(iface, statsType, statsFlags)


/**
 * ��ʼ/ֹͣ��¼ͨ���¼�
 *
 * @param[in] iface P2Pͨ��ʵ��
 * @param[in] filename ��־�ļ���,����ֹͣ��¼
 * @param[in] max_size_mb ��־�ļ������СMB
 *
 * @return �ɹ�����0
 *
 * @remarks
 *  ��־�ļ����� rtc_event_log.proto ����Ķ����Ƹ�ʽ,
 *  �����������߽���.
 */
#define JSEP_LogRtcEvent(iface, filename, max_size_mb) \
    JsepAPI(JSEP_API_LEVEL)->LogRtcEvent(iface, filename, max_size_mb)

/**
 * ��ʼ/ֹͣ��¼��Ƶ�������
 *
 * @param[in] filename ��־�ļ���,����ֹͣ��¼
 * @param[in] max_size_mb ��־�ļ������СMB
 *
 * @return �ɹ�����0
 *
 * @remarks
 *  ��־�ļ����� debug.proto ����Ķ����Ƹ�ʽ,
 *  �����������߽���.
 */
#define JSEP_DumpAudioProcessing(filename, max_size_mb) \
    JsepAPI(JSEP_API_LEVEL)->DumpAudioProcessing(filename, max_size_mb)

/**
 * �ֹ����÷�������
 *
 * @param[in] iface P2Pͨ��ʵ��
 * @param[in] current_bitrate_bps  ��ǰ����,��������0,������
 * @param[in] max_bitrate_bps ������������,��������0,������
 * @param[in] min_bitrate_bps �������С����,��������0,������
 *
 * @return �ɹ�����0
 */
#define JSEP_SetBitrate(iface, current_bitrate_bps, max_bitrate_bps, min_bitrate_bps) \
    JsepAPI(JSEP_API_LEVEL)->SetBitrate(iface, current_bitrate_bps, max_bitrate_bps, min_bitrate_bps)

/** @} */
///////////////////////////////////////////////////////////////////////////////
/**
 * @defgroup RTCSocket ����
 * @{
 */

/**
 * ���� WebSocket �ͻ��˲����ӵ�������
 * WebSocket �ǿɿ���TCP����
 * 
 * @param[in] wsURL WebSocket��������ַ,����'ws://example.com/path:7000' �� 'wss://192.168.0.22:7000'
 * @param[in] rtcWebSocketInit ���ò���, @see RTCWebSocketInit
 * @param[in] observer ������
 * @param[in] callback ͳһ�Ļص�����,��Windowsƽ̨�ⶼ�������߳��лص�
 *
 * @return ����WebSocketʵ��,ʧ�ܷ���nullptr
 *
 * @remarks
 *  �ͷ�ʵ�� ��ʹ��RTCSocket_Close()
 *  callback��Чʱ,��observer��Ϊ�����,������ʹ��,����ֱ�ӻص�observer.
 */
#define WebSocket_Connect(wsURL, rtcWebSocketInit, observer, callback) \
    JsepAPI(JSEP_API_LEVEL)->CreateWebSocket(wsURL, rtcWebSocketInit, observer, callback)

/**
 * ���� WebSocket �������������ͻ�������
 * WebSocket �ǿɿ���TCP����
 * 
 * @param[in] wsURL ������ַ,����'ws://:7000' �� 'wss://192.168.0.22:7000'
 * @param[in] rtcWebSocketInit ���ò���, @see RTCWebSocketInit
 * @param[in] observer ������
 * @param[in] callback ͳһ�Ļص�����,��Windowsƽ̨�ⶼ�������߳��лص�
 *
 * @return ����WebSocketʵ��,ʧ�ܷ���nullptr
 *
 * @remarks
 *  ֹͣ�������ͷ�ʵ�� ��ʹ��RTCSocket_Close()
 *  callback��Чʱ,��observer��Ϊ�����,������ʹ��,����ֱ�ӻص�observer.
 *  �����������ӵĸı�,�������� RTCSocketEvent_StateChange, ���� rtcsocket Ϊ�����ӵķ���ʵ��
 *      - �յ��µ�����, state �� 'new', 
 *      - ���ӽ����ɹ�, state �� 'open[: protocol]'
 *      - ���ӶϿ�, state �� 'closed', ��ʱ��Ҫ���� RTCSocket_Close(rtcsocket) �رո�����
 *  ������,�յ�����, ������ RTCSocketEvent_Message, ���� rtcsocket Ϊ�����ӵķ���ʵ��
 */

#define WebSocket_Listen(wsURL, rtcWebSocketInit, observer, callback) \
    JsepAPI(JSEP_API_LEVEL)->CreateWebSocketServer(wsURL, rtcWebSocketInit, observer, callback)

/**
 * ���� IceSocket
 *
 * IceSocket �ǲ��ɿ��Ķ�·�� P2P ����
 * 
 * @param[in] rtcConfiguration  STUN/TRUN ����������, @see RTCConfiguration 
 * @param[in] rtcIceParameters  ICE����,@see RTCIceParameters
 * @param[in] isCaller  �Ƿ����з�
 * @param[in] observer ������
 * @param[in] callback ͳһ�Ļص�����,��Windowsƽ̨�ⶼ���ڶ����߳��лص�
 *
 * @return ����IceSocketʵ��,ʧ�ܷ���nullptr
 *
 * @remarks
 *  �ͷ�ʵ�� ��ʹ��RTCSocket_Close()
 *  callback��Чʱ,��observer��Ϊ�����,������ʹ��,����ֱ�ӻص�observer.
 */
#define IceSocket_Connect(rtcConfiguration, rtcIceParameters, isCaller, observer, callback) \
    JsepAPI(JSEP_API_LEVEL)->CreateIceSocket(rtcConfiguration, rtcIceParameters, isCaller, observer, callback)

/**
 * �ر� RTCSocket
 *
 * @param[in] rtcSocket RTCSocketʵ��, ִ�к�,�ö���ʧЧ.
 * @return ���ش���ʱ���õ� observer
 */
#define RTCSocket_Close(rtcSocket) \
    JsepAPI(JSEP_API_LEVEL)->CloseSocket(rtcSocket)

/**
 * ������Ϣ
 *
 * @param[in] rtcSocket RTCSocketʵ��
 * @param[in] message ��Ϣ
 * @param[in] length ��Ϣ����,������,���ʾmessage�ַ�������
 *
 * @return �ɹ����ط��ͳ���,��֮������
 *
 * @remarks
 */
#define RTCSocket_Send(rtcSocket, message, length) \
    JsepAPI(JSEP_API_LEVEL)->SendSocket(rtcSocket, message, length)

/**
 * ��ӶԶ˵�ICE��ַ
 *
 * @param[in] iceSocket IceSocketʵ��
 * @param[in] candidate �Զ˵�ַ
 *
 * @return �ɹ�����0,��֮������
 */
#define IceSocket_AddRemoteCandidate(iceSocket, candidate) \
    JsepAPI(JSEP_API_LEVEL)->AddSocketIceCandidate(iceSocket, candidate)

/**
 * ���öԶ˵�ICE����
 *
 * @param[in] iceSocket IceSocketʵ��
 * @param[in] rtcIceParameters �Զ˲���, @see RTCIceParameters
 *
 * @return �ɹ�����0,��֮������
 */

#define IceSocket_SetRemoteParameters(iceSocket, rtcIceParameters) \
    JsepAPI(JSEP_API_LEVEL)->SetSocketIceParameters(iceSocket, rtcIceParameters)

/** @} */
///////////////////////////////////////////////////////////////////////////////
/**
 * @defgroup JSON ����
 * @{
 */
enum JsonForm {
    /** ��������,�߱�ԭ���Ե�JSON���� */
    JsonForm_Primitive      = 1,
    /** �ַ��� */
    JsonForm_String         = 2,
    /** ������� */
    JsonForm_Array          = 3,
    /** ���ϵ��ֵ���� */
    JsonForm_Object         = 4,
};

enum JsonError {
    /** JSON�������������tokens, ���Ȳ��� */
    JsonError_Insufficient = -1,
    /** �Ƿ���JSON�ַ��� */
    JsonError_Invalid    = -2,
    /** ����������JSON�ַ��� */
    JsonError_Partial    = -3,
};
/** ������.  ������0,��ʼ�� */
typedef struct { int _[3]; } JsonParser;
typedef struct JsonValue  JsonValue;

/**
 * ���� JSON �ַ���
 * ���뱣֤ ���㹻���ȵ� jsonValue
 *
 * @param[in] json �ַ���
 * @param[in] jsonValue Ԥ��������
 *
 * @return �ɹ����ظ���, ���ش���ֵ
 */
#define Json_ParseString(json, jsonValue) \
    JsepAPI(JSEP_API_LEVEL)->ParseJson(json, 0, (JsonParser*)0, jsonValue, sizeof(jsonValue)/sizeof(JsonValue));

/**
 * ������Ľ���
 *
 * @param[in] json �ַ���
 * @param[in] n_json �ַ�������
 * @param[in] jsonParser ����״̬
 * @param[in] jsonValue Ԥ��������
 * @param[in] size Ԥ�������
 * 
 * @return �ɹ����ظ���,���ش���ֵ
 *
 * @example 
 *  JsonValue token[256], *root=token;
 *  int r, n= sizeof(token)/sizeof(JsonValue);
 *  {
 *      JsonParser parse={{0}};
 *      do {
 *          r = Json_Parse(json, n_json, &parse, root, n);
 *          if (r == JsonError_Insufficient) {//Ԥ���䲻��
 *              n *= 2;
 *              if (root == token)
 *                  root = (JsonValue*)memcpy(malloc(sizeof(JsonValue)*n), token, sizeof(token));
 *              else
 *                  root = (JsonValue *)realloc(t, sizeof(JsonValue)*n);
 *          }
 *          else if (r > 0)
 *              break;
 *          else {//����
 *              if (root != token) free(root);
 *              return false;
 *          }
 *      } while(1);
 *  }
 *  .......
 *
 *  if (root != token) free(root);
 *  return true;
 */
#define Json_Parse(json, n_json, jsonParser, jsonValue, size) \
    JsepAPI(JSEP_API_LEVEL)->ParseJson(json, n_json, jsonParser, jsonValue, size)

/**
 * ���� JSON ��Ӧ���ַ���
 * �Զ�����ת�����
 *
 * @param[in] jsonValue JSON ����
 * @param[out] buf  ������ַ���,�������㹻���� jsonValue->n_json
 * @return ����д��ĳ���, ����ĩβ'\0'
 */
#define Json_Unescape(jsonValue, buf) \
    JsepAPI(JSEP_API_LEVEL)->UnescapeJson(jsonValue, buf)

/**
 * JSON �������ַ����Ƚ�
 * �Զ�����ת�����
 *
 * @param[in] jsonValue JSON ����
 * @param[in] str �ַ���
 * @return ����strcmp, 0��ʾ���
 */
#define Json_Compare(jsonValue, str) \
    JsepAPI(JSEP_API_LEVEL)->CompareJson(jsonValue, str, 0)

/** ͬ��, ����ָ���ַ������� */
#define Json_Compare2(jsonValue, str, n_str) \
    JsepAPI(JSEP_API_LEVEL)->CompareJson(jsonValue, str, n_str)

/**
 * ���� JSON ������ָ��Ԫ��
 *
 * @param[in] jsonValue JSON �����������
 * @param[in] index  Ԫ�ص����(��0��ʼ), -1��ʾĩβ
 *
 * @return ��Ӧ�� JSON ����, ʧ���򷵻���Ч����, �϶������� nullptr
 */
#define Json_Child(jsonValue, index) \
    JsepAPI(JSEP_API_LEVEL)->ChildJson(jsonValue, index, (const char*)0, 0)

/**
 * ���� JSON ���йؼ��ʶ�Ӧ��ֵ
 *
 * @param[in] jsonValue JSON �����
 * @param[in] key  �ؼ���
 *
 * @return ��Ӧ�� JSON ����, ʧ���򷵻���Ч����, �϶������� nullptr
 */
#define Json_Value(jsonValue, key) \
    JsepAPI(JSEP_API_LEVEL)->ChildJson(jsonValue, 0, key, 0)

/** ͬ��, ����ָ���ؼ��ʳ��� */
#define Json_Value2(jsonValue, key, n_key) \
    JsepAPI(JSEP_API_LEVEL)->ChildJson(jsonValue, 0, key, n_key)

/** @} */
///////////////////////////////////////////////////////////////////////////////
/** C�������÷�ʽ */
#if __GNUC__
#define JSEP_CDECL_CALL
#else
#define JSEP_CDECL_CALL __cdecl
#endif
typedef struct {
    // media stream
    int (JSEP_CDECL_CALL *AddLocalStream) (RTCPeerConnection* iface, const char* streamId, RTCBoolean* bAudio, RTCBoolean *bVideo, MediaStreamConstraints constraints);
    void (JSEP_CDECL_CALL *RemoveLocalStream) (RTCPeerConnection* iface, const char* streamId);
    int (JSEP_CDECL_CALL *PublishRemoteStream)(RTCPeerConnection* iface, const char* streamId, int renderOrCapturerBits, int videoTrackMask);

    // data channel
    int (JSEP_CDECL_CALL *CreateDataChannel) (RTCPeerConnection* iface, const char* channelId, RTCDataChannelInit rtcDataChannelInit);
    void (JSEP_CDECL_CALL *CloseDataChannel) (RTCPeerConnection* iface, const char* channelId);
    int (JSEP_CDECL_CALL *SendMessage) (RTCPeerConnection* iface, const char* channelId, const char* buffer, int length);

    // peer
    RTCPeerConnection* (JSEP_CDECL_CALL *CreatePeerConnection)(RTCConfiguration rtcConfiguration, RTCBoolean zmfAudioPump, RTCBoolean isCaller, RTCSessionObserver* userdata,
        void (JSEP_CDECL_CALL *observer)(RTCSessionObserver*userdata, enum RTCSessionEvent event, const char* json, int length));
    RTCSessionObserver* (JSEP_CDECL_CALL *ReleasePeerConnection) (RTCPeerConnection* iface);

    // negotiation
    int (JSEP_CDECL_CALL *CreateAnswer) (RTCPeerConnection* iface, RTCAnswerOptions rtcAnswerOptions);
    int (JSEP_CDECL_CALL *CreateOffer) (RTCPeerConnection* iface, RTCOfferOptions rtcOfferOptions);
    int (JSEP_CDECL_CALL *AddIceCandidate) (RTCPeerConnection* iface, RTCIceCandidate rtcIceCandidate);
    int (JSEP_CDECL_CALL *SetLocalDescription) (RTCPeerConnection* iface, RTCSessionDescription rtcSessionDescription);
    int (JSEP_CDECL_CALL *SetRemoteDescription) (RTCPeerConnection* iface, RTCSessionDescription rtcSessionDescription);

    // dynamic setting
    int (JSEP_CDECL_CALL *InsertDtmf) (RTCPeerConnection* iface, const char* tones, int duration_ms, int inter_tone_gap);
    int (JSEP_CDECL_CALL *SetBitrate)(RTCPeerConnection* iface, int current_bitrate_bps, int max_bitrate_bps, int min_bitrate_bps);

    // debug
    int (JSEP_CDECL_CALL *GetStats) (RTCPeerConnection* iface, const char* statsType, int statsFlags);
    int (JSEP_CDECL_CALL *LogRtcEvent)(RTCPeerConnection* iface, const char* filename, int max_size_mb);
    int (JSEP_CDECL_CALL *DumpAudioProcessing)(const char* filename, int max_size_mb);

    // socket
    RTCSocket* (JSEP_CDECL_CALL *CreateWebSocket)(const char* wsURL, RTCWebSocketInit rtcWebSocketInit, RTCSocketObserver* userdata,
        void (JSEP_CDECL_CALL *observer)(RTCSocketObserver* userdata, RTCSocket* rtcsocket, const char* message, int length, enum RTCSocketEvent event));
    RTCSocket* (JSEP_CDECL_CALL *CreateWebSocketServer)(const char* wsURL, RTCWebSocketInit rtcWebSocketInit, RTCSocketObserver* userdata,
        void (JSEP_CDECL_CALL *observer)(RTCSocketObserver* userdata, RTCSocket* rtcsocket, const char* message, int length, enum RTCSocketEvent event));
    RTCSocket* (JSEP_CDECL_CALL *CreateIceSocket)(RTCConfiguration rtcConfiguration, RTCIceParameters rtcIceParameters, RTCBoolean isCaller, RTCSocketObserver* userdata,
        void (JSEP_CDECL_CALL *observer)(RTCSocketObserver* userdata, RTCSocket* rtcsocket, const char* message, int length, enum RTCSocketEvent event));

    RTCSocketObserver* (JSEP_CDECL_CALL *CloseSocket)(RTCSocket* rtcsocket);
    int (JSEP_CDECL_CALL *SendSocket)(RTCSocket* rtcsocket, const char* message, int length);

    // ice socket
    int (JSEP_CDECL_CALL *AddSocketIceCandidate)(RTCSocket* rtcsocket, const char* candidate);
    int (JSEP_CDECL_CALL *SetSocketIceParameters)(RTCSocket* rtcsocket, RTCIceParameters rtcIceParameters);

    // json
    int (JSEP_CDECL_CALL *ParseJson)(const char *json, int len, JsonParser *parser, JsonValue* tokens, int n_tokens);
    const JsonValue* (JSEP_CDECL_CALL *ChildJson)(const JsonValue* jsonValue, int index, const char* key, int n_key);
    int (JSEP_CDECL_CALL *CompareJson)(const JsonValue* jsonValue, const char* str, int n_str);
    int (JSEP_CDECL_CALL *UnescapeJson)(const JsonValue* jsonValue, char* buf);
} JSEP_API;

#ifdef __cplusplus
extern "C" {
#endif
#ifdef JSEP_EXPORT
#if defined _WIN32 || defined __CYGWIN__
__declspec(dllexport) const JSEP_API* JSEP_CDECL_CALL JsepAPI(int apiLevel);
#elif __GNUC__ >= 4
__attribute__ ((visibility ("default"))) const JSEP_API* JSEP_CDECL_CALL JsepAPI(int apiLevel);
#endif
#elif defined JSEP_IMPORT
#if __GNUC__ >= 4
__attribute__ ((visibility ("default")))
#endif
const JSEP_API* JSEP_CDECL_CALL JsepAPI(int apiLevel);
#else
__inline const JSEP_API* JsepAPI(int apiLevel) {
    typedef const JSEP_API* (JSEP_CDECL_CALL *PFN_JSEPAPI)(int);
    static PFN_JSEPAPI pfn = (PFN_JSEPAPI) 0;
    if (!pfn) do {
        void* handle = 0;
        char folderpath[2048] = {0};
#ifdef _WIN32
        strcpy(folderpath, "jsep.dll");
        handle = (void*)LoadLibraryA(folderpath);
#else
        strcpy(folderpath, "libjsep.so");
        handle = (void*)dlopen(folderpath, RTLD_LOCAL|RTLD_LAZY);
#endif
        if (!handle) {
#ifdef __linux__
            if (readlink ("/proc/self/exe", folderpath, sizeof(folderpath)) != -1) {
                char* p = strrchr(folderpath, '/');
                strcpy(p, "/libjsep.so");
                handle = (void*)dlopen(folderpath, RTLD_LOCAL|RTLD_LAZY);
            }
#elif defined __APPLE__
            char *p;
            char pathbuf[PROC_PIDPATHINFO_MAXSIZE];
            char folderpath[PROC_PIDPATHINFO_MAXSIZE];
            pid_t pid = getpid();
            proc_pidpath (pid, pathbuf, sizeof(pathbuf));
            realpath(pathbuf, folderpath);
            p = strrchr(folderpath, '/');
            strcpy(p, "/libjsep.so");
            handle = (void*)dlopen(folderpath, RTLD_LOCAL|RTLD_LAZY);
            if (!handle) {
                strcpy(p, "/../Frameworks/libjsep.so");
                handle = (void*)dlopen(folderpath, RTLD_LOCAL|RTLD_LAZY);
            }
#endif
        }
        if (handle) {
#ifdef _WIN32
            pfn = (PFN_JSEPAPI)GetProcAddress((HMODULE)handle, "JsepAPI");
#else
            pfn = (PFN_JSEPAPI)dlsym(handle, "JsepAPI");
#endif
        }
        if (!pfn){
            void* str;
#ifndef _WIN32
            str = (void*)dlerror();
#else
            FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&str, 0, NULL);
            OutputDebugStringA((LPCSTR)str);
#endif
            fprintf(stderr, "\n%s\nUnable to load JSEP library: %s\n", (char*)str, folderpath);
            return (const JSEP_API*)0;
        }
    } while(0);
    return pfn(apiLevel);
}
#endif //JSEP_EXPORT
#ifdef __cplusplus
}
#endif
///////////////////////////////////////////////////////////////////////////////
/**
 * JSON���������.
 * @param[in] type ����, ������JSON_OBJECT, JSON_ARRAY, JSON_STRING ֮һ
 * @param[in] start ��������JSON�ַ�������ʼλ��
 * @param[in] end ��������JSON�ַ����Ľ���λ��
 */
struct JsonValue {
    int /*enum JsonForm*/ type;
    const char* json;
    int n_json;
    int n_child;
    int parent;
#ifdef __cplusplus
    int compare(const std::string& str) const { return JsepAPI(JSEP_API_LEVEL)->CompareJson(this, str.data(), str.length()); }
    const JsonValue& operator[](const std::string& key) const { return *(JsepAPI(JSEP_API_LEVEL)->ChildJson(this, 0, key.data(), key.length())); }
    const JsonValue& operator[](int index) const { return *(JsepAPI(JSEP_API_LEVEL)->ChildJson(this, index, 0, 0)); }
    bool operator==(const std::string& str) const { return compare(str) == 0; }
    bool operator!=(const std::string& str) const { return compare(str) != 0; } 
    operator std::string() const { return str(); }
    static std::string escape (const std::string& str){
        std::string ret;
        ret.reserve(str.length());
        for (std::string::size_type i = 0; i < str.length(); ++i) {
            switch (str[i]) {
            case '"': ret += "\\\""; break;
            case '\b': ret += "\\b"; break;
            case '\f': ret += "\\f"; break;
            case '\n': ret += "\\n"; break;
            case '\r': ret += "\\r"; break;
            case '\t': ret += "\\t"; break;
            case '\\': ret += "\\\\"; break;
            default: ret += str[i]; break;
            }
        }
        return ret;
    }
    static const std::vector<JsonValue> parse(const char* json, int n_json=0) {
        int ret;
        JsonParser p={{0}};
        std::vector<JsonValue> vals(256);
        do {
            ret = JsepAPI(JSEP_API_LEVEL)->ParseJson(json, n_json, &p, &vals[0], vals.size());
            if (ret == JsonError_Insufficient)
                vals.resize(vals.size()*2);
            else{
                if (ret < 0) vals.clear();
                break;
            }
        } while (1);
        return vals;
    }
    const std::string str() const {
        if (type != JsonForm_String)
            return std::string(json, n_json);
        else {
            int i = 0;
            while (i<n_json && json[i]!='\\') ++i;
            if (i == n_json)
                return std::string(json, n_json);
            else {
                std::string ret(json, i);
                do {
                    char ch = json[i++];
                    if (ch == '\\') {
                        switch(json[i++]) {
                        case '\\':ch = '\\'; break;
                        case '"': ch = '\"'; break;
                        case 'f': ch = '\f'; break;
                        case 'n': ch = '\n'; break;
                        case 'r': ch = '\r'; break;
                        case 't': ch = '\t'; break;
                        case '0': ch = '\0'; break;
                        }
                    }
                    ret.push_back(ch);
                } while (i < n_json);
                return ret;
            }
        }
    }
#endif //__cplusplus
};
#endif
