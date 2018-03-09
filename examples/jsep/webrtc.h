#ifndef __WEBRTC_H__
#define __WEBRTC_H__
#pragma once
#include <string>
/**
 *  ʹ��JSEP������ҳWebRTC��ͨ
 * 
 *  ʵ��ƽ̨��صĴ�����,�ر���UI�̻߳ص�����.
 *      class MyWindow : public WebRTC::View {
 *          ......
 *          void _QueueUIThreadCallback(int msg_id, char* data){......}
 *      };
 *  ��������:
 *      - ����
 *        MyWindow& view = ...;
 *        WebRTC* wc = WebRTC::Connect(view, addr, id);
 *      - �ȴ����Ӵ��¼�
 *        WebRTC::View::OnOpen() 
 *      - ����ͨ��
 *        wc->Call(......);
 *      - �Ҷ�ͨ��
 *        wc->Hangup();
 *      - �Ͽ�����
 *        wc->Close();
 *        wc = 0;
 */
class WebRTC {
public:
    class View {
    public:
        //�����ѽ���
        virtual void OnOpen() = 0;
        //�����ѹر�
        virtual void OnClose() = 0;
        //�յ�����
        virtual void OnCall(const std::string& fromId, bool p2pOnly) = 0;
        //���Ҷ�
        virtual void OnHangup() = 0;

        /**
         * �յ���Ϣ
         *
         * @param[in] message ��Ϣ�ı�
         * @param[in] fromId  ������
         */
        virtual void OnMessage(const std::string& message, const std::string& fromId) = 0;

        /**
         * ���ý����
         * @param[in] streamId  ��ID
         * @param[in] type ������
         *
         * @remarks
         *       type ��ѡΪ
         *       - 'previewaudio', ��ƵԤ��
         *       - 'previewshare', ����Ԥ��
         *       - 'localmedia', ��������Ƶ
         *       - 'localshare', ���ع���
         *       - 'peervideo', �Զ���Ƶ
         *       - 'peeraudio', �Զ���Ƶ
         *       - 'peershare', �Զ˹���
         *       - 'peerdata',  �Զ˷�ý������
         */
        virtual void OnAddStream(const std::string& streamId, const std::string& type) = 0;

        //ɾ��ý����
        virtual void OnRemoveStream(const std::string& streamId, const std::string& type) = 0;

        /**
         * ͳ�Ʊ���
         * 
         * @param[in] statsType ͳ������
         * @param[in] statsId ͳ��ID
         * @param[in] stats ��ϸ����,JSON��ʽ
         * @param[in] timestamp ʱ���
         * 
         * @remarks
         * ͳ������, statsType ��������: UTF8�ַ���, ��ѡֵ
         *      - googLibjingleSession  ȫ�ֻỰ
         *      - transport             �����
         *      - VideoBwe              ��Ƶ�������
         *      - remoteSsrc            �Զ�RTP��
         *      - ssrc                  RTP��
         *      - googTrack             ý����
         *      - localcandidate        ����ICE
         *      - remotecandidate       �Զ�ICE
         *      - googComponent
         *      - googCandidatePair
         *      - googCertificate
         *      - datachannel           ����ͨ��
         */
        virtual void OnStatsReport(const std::string& statsTyep, const std::string& statsId,
            const std::string& stats, const double timestamp) = 0;

        //�ڲ���ӡ
        virtual void Trace(const char* format, ...) = 0;

        //������ת��UI�̺߳�,�ٵ���WebRTC::_UIThreadCallback()
        virtual void _QueueUIThreadCallback(int msg_id, char* data) = 0;
    protected:
        virtual ~View() {}
    };

public:
    /**
     * �ر�����
     */
    virtual void Close() = 0;

    /**
     * ������Ϣ
     */
    virtual void Send(const std::string& message, const std::string& toId) = 0;


    /**
     * ����P2P ����
     *
     * @param[in] configuration  ��׼ RTCConfiguration ����
     * @param[in] toId Ŀ���û���
     *
     * @return ʧ�ܷ���false
     */
    virtual bool P2P(const std::string& configuration, const std::string& toId) = 0;

    /**
     * ����ͨ��
     *
     * @param[in] configuration  ��׼ RTCConfiguration ����
     * @param[in] constraints  ��׼ MediaStreamConstraints ����
     * @param[in] toId ����Ŀ���û���
     *
     * @return ʧ�ܷ���false
     */
    virtual bool Call(const std::string& configuration, const std::string& constraints, const std::string& toId) = 0;

    /**
     * ���±���ý��
     * @param[in] constraints  ��׼ MediaStreamConstraints ����
     *
     * @return ʧ�ܷ���false
     */
    virtual bool Update(const std::string& constraints) = 0;

    /**
     * ���ع�����Ļ
     * @param[in] constraints  ��׼ MediaStreamConstraints ����
     *
     * @return ʧ�ܷ���false
     */
    virtual bool Share(const std::string& constraints) = 0;

    /*
     * ��ʼ/ֹͣ��¼ͨ���¼�
     *
     * @param[in] filename ��־�ļ���,����ֹͣ��¼
     */
    virtual void LogRtcEvent(const std::string& filename) = 0;

    /*
     * ��ʼ/ֹͣ��¼��Ƶ�������
     *
     * @param[in] filename ��־�ļ���,����ֹͣ��¼
     */
    virtual void DumpAudioProcessing(const std::string& filename) = 0;

    /**
     * �Ҷ�ͨ��
     */
    virtual void Hangup() = 0;

    /**
     * ����DTMF
     *
     * @param[in] tones DMTF������,0-9,A-D��a-d,#,*. �����޷�ʶ����ַ�.
     * @param[in] duration ÿ�������ĳ�������MS,���ܳ���6000��С��70
     * @param[in] inter_tone_gap �������,����Ϊ50ms,��Ӧ�����ܶ�
     *
     * @return �ɹ�����true
     *
     * @remarks
     *  ���л�����DTMF ����.
     *  ','��ʾ�ӳ�2�봦����һ���ַ�
     *  ������ʱ,�ϴ���������,��֮ǰ�����񽫱�ȡ��
     */
    virtual bool InsertDtmf(const std::string& tones, int duration, int inter_tone_gap) = 0;

    /**
     * GetStats�ӿڵĲ���
     * ����ϸ�ֻ����ͳ�ƽ��
     */
    enum {
        /** ��ϸ��, ���Լ����ͳ�� */
        Debug = 1,
        /** ������Ƶ��ص�ͳ�� */
        Audio = 2,
        /** ������Ƶ��Ӧ��ͳ�� */
        Video = 4,
    };

    /**
     * ��ȡͳ��
     *
     * @param[in] statsType ͳ�����,""��ʾ��ȡ����ͳ��
     * @param[in] bDebug  �Ƿ����ϸ�ĵ��Լ���
     *
     * @return ʧ�ܷ���false,��֮�ȴ�OnStatsReport �¼�.
     */
    virtual bool GetStats(const std::string& statsType, int statsFlags) = 0;

    /**
     * ��View ��UI�߳��лص�
     *
     * msg_id, data ��Ӧ View::_QueueUIThreadCallback()�в���.
     */
    virtual void _UIThreadCallback(int msg_id, char* data) = 0;

public:
    /**
     * ����P2P ����, �����򻯰�
     *
     * @param[in] stunURL  STUN/TURN ��������ַ
     * @param[in] password  �������������������
     * @param[in] toId Ŀ���û���
     *
     * @return ʧ�ܷ���false
     */
    virtual bool P2P(const std::string& stunURL, const std::string& password, const std::string& toId) = 0;

    /**
     * ����ͨ��, �����򻯰�
     *
     * @param[in] stunURL  STUN/TURN ��������ַ
     * @param[in] password  �������������������
     * @param[in] video  �Ƿ���Ƶ
     * @param[in] toId ����Ŀ���û���
     *
     * @return ʧ�ܷ���false
     */
    virtual bool Call(const std::string& stunURL, const std::string& password, bool video, const std::string& toId) = 0;

    /**
     * ���±���ý��, �����򻯰�
     * @param[in] video  �Ƿ���Ƶ
     *
     * @return ʧ�ܷ���false
     */
    virtual bool Update(bool video) = 0;

    /**
     * ���ع�����Ļ,�����򻯰�
     */
    virtual bool Share(bool share) = 0;

    /**
     * ����WebSocket������
     *
     * @param[in] view ��ʾ����������
     * @param[in] server ��������ַ,��ʽ����"ws://192.168.0.240:7000"
     * @param[in] myId  �û���
     *
     * @return ʧ�ܷ���NULL, ��֮�ȴ�View::OnOpen()/OnClose�¼�
     */
    static WebRTC* Connect(View& view, const std::string& server, const std::string& myId);

protected:
    virtual ~WebRTC() {}
};
#endif
