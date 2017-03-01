
#ifndef __TESTA_OBJECTHOLDER_H__
#define __TESTA_OBJECTHOLDER_H__

#include "IPC/cplusplus/lib/include/IpcException.h"
#include "IPC/cplusplus/lib/include/ObjectHolder.h"
#include "AXP/cplusplus/libc/include/Common/ClassLoader.h"
#include "IPC/gen/TestA/ITestA.h"

namespace TestA
{
    class AObjectHolder : public IPC::CObjectHolder
    {
    public:

        AXP::Sp<AXP::CParcel> OnTransact(
            IN CONST AXP::Sp<AXP::CParcel> & parcel,
            IN CONST AXP::Sp<AXP::String> & uri)
        {
            if (parcel == NULL)
                return NULL;

            AXP::Int32 funCode;
            if (AXP::AFAILED(parcel->ReadInt32(funCode)))
                return NULL;

            if (funCode == TestA_A_A) {
                if (mService == NULL) {
                    Synchronized (&mLock) {
                        if (mService == NULL) {
                            mService = _A::Create();
                            if (mService == NULL)
                                return NULL;
                        }
                    }
                }

                parcel->Reset();
                if (AXP::AFAILED(parcel->WriteInt32((AXP::Int32)IPC::NoException)))
                    return NULL;

                if (AXP::AFAILED(parcel->WriteInt64((AXP::Int64)this)))
                    return NULL;

                if (AXP::AFAILED(parcel->WriteInt64((AXP::Int64)(IA*)mService.Get())))
                    return NULL;
            }
            else {
                if (mService == NULL) {
                    parcel->Reset();
                    if (AXP::AFAILED(parcel->WriteInt32((AXP::Int32)IPC::RemoteRefException)))
                        return NULL;

                    return parcel;
                }

                if (funCode == TestA_A_Foo) {
                    AXP::Int32 a;
                    if (AXP::AFAILED(parcel->ReadInt32(a)))
                        return NULL;

                    AXP::Sp<NA::CList> b;
                    if (AXP::AFAILED(AXP::Libc::Common::ClassLoader::ReadObjectFromParcel(parcel, b)))
                        return NULL;

                    AXP::Int32 ret = mService->Foo(a, b);
                    parcel->Reset();
                    if (AXP::AFAILED(parcel->WriteInt32((AXP::Int32)IPC::NoException)))
                        return NULL;

                    if (AXP::AFAILED(parcel->WriteInt32(ret)))
                        return NULL;
                }
                else if (funCode == TestA_A_SetListener) {
                    AXP::Int64 _bb;
                    if (AXP::AFAILED(parcel->ReadInt64(_bb)))
                        return NULL;

                    AXP::Sp<AXP::String> token;
                    if (AXP::AFAILED(parcel->ReadString(token)))
                        return NULL;

                    AXP::Sp<TestB::B> bb;
                    try {
                        bb = TestB::B::Create(_bb, token);
                    }
                    catch (std::exception & e) {
                        return NULL;
                    }

                    mService->SetListener(bb);
                    parcel->Reset();
                    if (AXP::AFAILED(parcel->WriteInt32((AXP::Int32)IPC::NoException)))
                        return NULL;
                }
            }

            return parcel;
        }

    private:

        AXP::Sp<_A> mService;

    private:

        STATIC CONST AXP::Int32 TestA_A_Foo = 0;
        STATIC CONST AXP::Int32 TestA_A_SetListener = 1;
        STATIC CONST AXP::Int32 TestA_A_A = 2;
    };

    AXP::Sp<IPC::CObjectHolder> CreateAObjectHolder()
    {
        return new AObjectHolder();
    }
}

#endif // __TESTA_OBJECTHOLDER_H__
