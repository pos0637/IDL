
#ifndef __TESTB_H__
#define __TESTB_H__

#include "IPC/cplusplus/lib/include/ServerConnection.h"
#include "IPC/cplusplus/lib/include/IpcException.h"
#include "IPC/cplusplus/lib/include/ServiceManager.h"
#include "./ITestB.h"

namespace TestB
{
    class B : public AXP::CObject, public IB
    {
    public:

        STATIC AXP::Sp<B> Create()
        {
            return new B();
        }

        STATIC AXP::Sp<B> Create(IN AXP::Int64 objRef, IN CONST AXP::Sp<AXP::String> & token)
        {
            return new B(objRef, token);
        }

    protected:

        B()
        {
            mTag = (AXP::Int8)0x83;
            mInterface = NULL;
            AXP::Sp<AXP::CParcel> parcel = new AXP::CParcel();
            if (parcel == NULL)
                throw std::exception();

            AXP::Sp<IPC::IStub> stub = IPC::ServiceManager::GetService(TestB_B_DESCRIPTOR);
            if (stub == NULL) {
                mIsRemote = TRUE;
                mConn = IPC::CServerConnection::Create(TestB_B_DESCRIPTOR);
                if (mConn == NULL)
                    throw std::exception();

                if (AXP::AFAILED(parcel->WriteInt8(mTag)))
                    throw std::exception();

                if (AXP::AFAILED(parcel->WriteString(mToken)))
                    throw std::exception();

                if (AXP::AFAILED(parcel->WriteString(TestB_B_DESCRIPTOR)))
                    throw std::exception();
            }
            else {
                mIsRemote = FALSE;
                mConn = stub;
            }

            if (AXP::AFAILED(parcel->WriteInt32(IPC::CommandCode::COMMAND_CREATE)))
                throw std::exception();

            if (AXP::AFAILED(parcel->WriteBoolean(mIsRemote)))
                throw std::exception();

            if (mIsRemote) {
                if (AXP::AFAILED(parcel->WriteString(IPC::ServiceManager::sServerAddress)))
                    throw std::exception();
            }

            if (AXP::AFAILED(parcel->WriteInt32(TestB_B_B)))
                throw std::exception();

            parcel->Reset();
            parcel = mConn->Transact(parcel);
            if (parcel == NULL)
                throw std::exception();

            parcel->Reset();
            AXP::Int32 code;
            if (AXP::AFAILED(parcel->ReadInt32(code)))
                throw std::exception();

            IPC::ReadException(code);
            if (AXP::AFAILED(parcel->ReadInt64(mRef)))
                throw std::exception();

            if (!mIsRemote) {
                AXP::Int64 serviceObject;
                if (AXP::AFAILED(parcel->ReadInt64(serviceObject)))
                    throw std::exception();

                mInterface = (IB*)serviceObject;
            }
        }

        B(IN AXP::Int64 objRef, IN CONST AXP::Sp<AXP::String> & token)
        {
            mTag = (AXP::Int8)0x84;
            mToken = token;
            mInterface = NULL;
            mIsRemote = TRUE;
            mRef = objRef;
            mConn = IPC::CServerConnection::Create(TestB_B_DESCRIPTOR);
            if (mConn == NULL)
                throw std::exception();

        }

    public:

        VIRTUAL ~B()
        {
            if (mToken)
                return;
            try {
                AXP::Sp<AXP::CParcel> parcel = new AXP::CParcel();
                if (parcel == NULL)
                    throw std::exception();

                if (mIsRemote) {
                    if (AXP::AFAILED(parcel->WriteInt8(mTag)))
                        throw std::exception();

                    if (AXP::AFAILED(parcel->WriteString(mToken)))
                        throw std::exception();

                    if (AXP::AFAILED(parcel->WriteString(TestB_B_DESCRIPTOR)))
                        throw std::exception();
                }

                if (AXP::AFAILED(parcel->WriteInt32(IPC::CommandCode::COMMAND_RELEASE)))
                    throw std::exception();

                if (AXP::AFAILED(parcel->WriteBoolean(mIsRemote)))
                    throw std::exception();

                if (mIsRemote) {
                    if (AXP::AFAILED(parcel->WriteString(IPC::ServiceManager::sServerAddress)))
                        throw std::exception();
                }

                if (AXP::AFAILED(parcel->WriteInt64(mRef)))
                    throw std::exception();

                parcel->Reset();
                parcel = mConn->Transact(parcel);
                if ((mTag & 0x0F) == 0x04)
                    return;

                if (parcel == NULL)
                    throw std::exception();

                parcel->Reset();
                AXP::Int32 code;
                if (AXP::AFAILED(parcel->ReadInt32(code)))
                    throw std::exception();

                IPC::ReadException(code);
            }
            catch(...) {
            }
        }

    public:

        AXP::Int32 Bar(IN AXP::Int32 a)
        {
            if (mIsRemote) {
                AXP::Sp<AXP::CParcel> parcel = new AXP::CParcel();
                if (parcel == NULL)
                    throw std::exception();

                if (AXP::AFAILED(parcel->WriteInt8(mTag)))
                    throw std::exception();

                if (AXP::AFAILED(parcel->WriteString(mToken)))
                    throw std::exception();

                if (AXP::AFAILED(parcel->WriteString(TestB_B_DESCRIPTOR)))
                    throw std::exception();

                if (AXP::AFAILED(parcel->WriteInt32(IPC::CommandCode::COMMAND_CALL)))
                    throw std::exception();

                if (AXP::AFAILED(parcel->WriteInt64(mRef)))
                    throw std::exception();

                if (AXP::AFAILED(parcel->WriteInt32(TestB_B_Bar)))
                    throw std::exception();

                if (AXP::AFAILED(parcel->WriteInt32(a)))
                    throw std::exception();

                parcel->Reset();
                parcel = mConn->Transact(parcel);
                if ((mTag & 0x0F) == 0x04)
                    return 0;

                if (parcel == NULL)
                    throw std::exception();

                parcel->Reset();
                AXP::Int32 code;
                if (AXP::AFAILED(parcel->ReadInt32(code)))
                    throw std::exception();

                IPC::ReadException(code);
                AXP::Int32 ret;
                if (AXP::AFAILED(parcel->ReadInt32(ret)))
                    throw std::exception();

                return ret;
            }
            else {
                return mInterface->Bar(a);
            }
        }

    public:

        AXP::Int64 GetRemoteRef()
        {
            return mRef;
        }

        AXP::Void AddRemoteRef(IN CONST AXP::Sp<AXP::String> & uri)
        {
            mConn->AddRemoteRef(uri, mRef);
        }

        IB * GetInterface()
        {
            return mInterface;
        }

    private:

        AXP::Int8 mTag;
        AXP::Boolean mIsRemote;
        AXP::Int64 mRef;
        AXP::Sp<IPC::IStub> mConn;
        AXP::Sp<AXP::String> mToken;
        AXP::Wp<IB> mInterface;

    private:

        STATIC CONST AXP::Int32 TestB_B_Bar = 0;
        STATIC CONST AXP::Int32 TestB_B_B = 1;
        CONST AXP::Sp<AXP::String> TestB_B_DESCRIPTOR = AXP::String::Create(L"TestB.B");
    };
}

#endif // __TESTB_H__
