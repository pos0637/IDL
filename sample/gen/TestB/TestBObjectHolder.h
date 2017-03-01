
#ifndef __TESTB_OBJECTHOLDER_H__
#define __TESTB_OBJECTHOLDER_H__

#include "IPC/cplusplus/lib/include/IpcException.h"
#include "IPC/cplusplus/lib/include/ObjectHolder.h"
#include "IPC/gen/TestB/ITestB.h"

namespace TestB
{
    class BObjectHolder : public IPC::CObjectHolder
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

            if (funCode == TestB_B_B) {
                if (mService == NULL) {
                    Synchronized (&mLock) {
                        if (mService == NULL) {
                            mService = _B::Create();
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

                if (AXP::AFAILED(parcel->WriteInt64((AXP::Int64)(IB*)mService.Get())))
                    return NULL;
            }
            else {
                if (mService == NULL) {
                    parcel->Reset();
                    if (AXP::AFAILED(parcel->WriteInt32((AXP::Int32)IPC::RemoteRefException)))
                        return NULL;

                    return parcel;
                }

                if (funCode == TestB_B_Bar) {
                    AXP::Int32 a;
                    if (AXP::AFAILED(parcel->ReadInt32(a)))
                        return NULL;

                    AXP::Int32 ret = mService->Bar(a);
                    parcel->Reset();
                    if (AXP::AFAILED(parcel->WriteInt32((AXP::Int32)IPC::NoException)))
                        return NULL;

                    if (AXP::AFAILED(parcel->WriteInt32(ret)))
                        return NULL;
                }
            }

            return parcel;
        }

    private:

        AXP::Sp<_B> mService;

    private:

        STATIC CONST AXP::Int32 TestB_B_Bar = 0;
        STATIC CONST AXP::Int32 TestB_B_B = 1;
    };

    AXP::Sp<IPC::CObjectHolder> CreateBObjectHolder()
    {
        return new BObjectHolder();
    }
}

#endif // __TESTB_OBJECTHOLDER_H__
