
#ifndef __TESTA_H__
#define __TESTA_H__

#include "IPC/cplusplus/lib/include/ServerConnection.h"
#include "IPC/cplusplus/lib/include/IpcException.h"
#include "IPC/cplusplus/lib/include/ServiceManager.h"
#include "./ITestA.h"
#include "AXP/cplusplus/libc/include/Common/ClassLoader.h"

namespace TestA
{
    class A : public AXP::CObject, public IA
    {
    public:

        STATIC AXP::Sp<A> Create()
        {
            return new A();
        }

        STATIC AXP::Sp<A> Create(IN AXP::Int64 objRef, IN CONST AXP::Sp<AXP::String> & token)
        {
            return new A(objRef, token);
        }

    protected:

        A()
        {
            mTag = (AXP::Int8)0x83;
            mInterface = NULL;
            AXP::Sp<AXP::CParcel> parcel = new AXP::CParcel();
            if (parcel == NULL)
                throw std::exception();

            AXP::Sp<IPC::IStub> stub = IPC::ServiceManager::GetService(TestA_A_DESCRIPTOR);
            if (stub == NULL) {
                mIsRemote = TRUE;
                mConn = IPC::CServerConnection::Create(TestA_A_DESCRIPTOR);
                if (mConn == NULL)
                    throw std::exception();

                if (AXP::AFAILED(parcel->WriteInt8(mTag)))
                    throw std::exception();

                if (AXP::AFAILED(parcel->WriteString(mToken)))
                    throw std::exception();

                if (AXP::AFAILED(parcel->WriteString(TestA_A_DESCRIPTOR)))
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

            if (AXP::AFAILED(parcel->WriteInt32(TestA_A_A)))
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

                mInterface = (IA*)serviceObject;
            }
        }

        A(IN AXP::Int64 objRef, IN CONST AXP::Sp<AXP::String> & token)
        {
            mTag = (AXP::Int8)0x84;
            mToken = token;
            mInterface = NULL;
            mIsRemote = TRUE;
            mRef = objRef;
            mConn = IPC::CServerConnection::Create(TestA_A_DESCRIPTOR);
            if (mConn == NULL)
                throw std::exception();

        }

    public:

        VIRTUAL ~A()
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

                    if (AXP::AFAILED(parcel->WriteString(TestA_A_DESCRIPTOR)))
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

        AXP::Int32 Foo(IN AXP::Int32 a, IN CONST AXP::Sp<NA::CList> & b)
        {
            if (mIsRemote) {
                AXP::Sp<AXP::CParcel> parcel = new AXP::CParcel();
                if (parcel == NULL)
                    throw std::exception();

                if (AXP::AFAILED(parcel->WriteInt8(mTag)))
                    throw std::exception();

                if (AXP::AFAILED(parcel->WriteString(mToken)))
                    throw std::exception();

                if (AXP::AFAILED(parcel->WriteString(TestA_A_DESCRIPTOR)))
                    throw std::exception();

                if (AXP::AFAILED(parcel->WriteInt32(IPC::CommandCode::COMMAND_CALL)))
                    throw std::exception();

                if (AXP::AFAILED(parcel->WriteInt64(mRef)))
                    throw std::exception();

                if (AXP::AFAILED(parcel->WriteInt32(TestA_A_Foo)))
                    throw std::exception();

                if (AXP::AFAILED(parcel->WriteInt32(a)))
                    throw std::exception();

                if (AXP::AFAILED(AXP::Libc::Common::ClassLoader::WriteObjectToParcel(parcel, b)))
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
                return mInterface->Foo(a, b);
            }
        }

        AXP::Void SetListener(IN CONST AXP::Sp<TestB::B> & bb)
        {
            if (mIsRemote) {
                AXP::Sp<AXP::CParcel> parcel = new AXP::CParcel();
                if (parcel == NULL)
                    throw std::exception();

                if (AXP::AFAILED(parcel->WriteInt8(mTag)))
                    throw std::exception();

                if (AXP::AFAILED(parcel->WriteString(mToken)))
                    throw std::exception();

                if (AXP::AFAILED(parcel->WriteString(TestA_A_DESCRIPTOR)))
                    throw std::exception();

                if (AXP::AFAILED(parcel->WriteInt32(IPC::CommandCode::COMMAND_CALL)))
                    throw std::exception();

                if (AXP::AFAILED(parcel->WriteInt64(mRef)))
                    throw std::exception();

                if (AXP::AFAILED(parcel->WriteInt32(TestA_A_SetListener)))
                    throw std::exception();

                if (AXP::AFAILED(parcel->WriteInt64(bb->GetRemoteRef())))
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
                bb->AddRemoteRef(IPC::ServiceManager::GetProxyAddr((AXP::PCWStr)*TestA_A_DESCRIPTOR));
                return;
            }
            else {
                mInterface->SetListener(bb);
                return;
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

        IA * GetInterface()
        {
            return mInterface;
        }

    private:

        AXP::Int8 mTag;
        AXP::Boolean mIsRemote;
        AXP::Int64 mRef;
        AXP::Sp<IPC::IStub> mConn;
        AXP::Sp<AXP::String> mToken;
        AXP::Wp<IA> mInterface;

    private:

        STATIC CONST AXP::Int32 TestA_A_Foo = 0;
        STATIC CONST AXP::Int32 TestA_A_SetListener = 1;
        STATIC CONST AXP::Int32 TestA_A_A = 2;
        CONST AXP::Sp<AXP::String> TestA_A_DESCRIPTOR = AXP::String::Create(L"TestA.A");
    };
}

#endif // __TESTA_H__
