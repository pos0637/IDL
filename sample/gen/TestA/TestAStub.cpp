
#include "IPC/cplusplus/lib/include/ServiceManager.h"
#include "IPC/cplusplus/lib/include/IpcException.h"
#include "IPC/cplusplus/lib/include/UriManager.h"

namespace TestA
{
    EXTERN AXP::Sp<IPC::CObjectHolder> CreateAObjectHolder();

    class AStub : public IPC::IStub
    {
    public:

        AStub()
        {
            mServiceList = new AXP::HashTable<AXP::Int64, IPC::CObjectHolder>(50);
        }

        AXP::Void AddRemoteRef(IN CONST AXP::Sp<AXP::String> & uri, IN AXP::Int64 objRef)
        {
            if (mServiceList == NULL)
                return;

            AXP::Sp<IPC::CObjectHolder> obj = mServiceList->GetValue(objRef);
            if (obj && uri) {
                IPC::UriManager::StartThread(uri);
                obj->mUriList->PushBack(uri);
                obj->AddRemoteRef();
            }
        }

        AXP::Sp<AXP::CParcel> Transact(IN CONST AXP::Sp<AXP::CParcel> & bundle)
        {
            if (bundle == NULL)
                return NULL;

            AXP::Int32 code;
            if (AXP::AFAILED(bundle->ReadInt32(code)))
                return NULL;

            AXP::Sp<AXP::String> uri = NULL;
            switch (code) {
            case IPC::CommandCode::COMMAND_CREATE:
            case IPC::CommandCode::COMMAND_RELEASE:
                AXP::Boolean isRemote;
                if (AXP::AFAILED(bundle->ReadBoolean(isRemote)))
                    return NULL;

                if (isRemote) {
                    if (AXP::AFAILED(bundle->ReadString(uri)))
                        return NULL;
                }

                break;
            case IPC::CommandCode::COMMAND_CALLBACK:
                if (AXP::AFAILED(bundle->ReadString(uri)))
                    return NULL;

                break;
            case IPC::CommandCode::COMMAND_CALL:
                break;
            default:
                return NULL;
            }

            AXP::Sp<AXP::CParcel> parcel;
            AXP::Sp<IPC::CObjectHolder> objectHolder = CreateOrGetObjectHolder(code, uri, bundle, parcel);
            if (objectHolder == NULL)
                return parcel;

            if (code == IPC::CommandCode::COMMAND_RELEASE) {
                if (AXP::AFAILED(ReleaseObjectHolder(objectHolder, uri, FALSE)))
                    return NULL;

                parcel = new AXP::CParcel();
                if (parcel == NULL)
                    return NULL;

                if (AXP::AFAILED(parcel->WriteInt32((AXP::Int32)IPC::NoException)))
                    return NULL;

                return parcel;
            }
            else if (code == IPC::CommandCode::COMMAND_CALLBACK) {
                parcel = new AXP::CParcel();
                if (parcel == NULL)
                    return NULL;

                if (AXP::AFAILED(parcel->WriteInt32((AXP::Int32)IPC::NoException)))
                    return NULL;

                if (AXP::AFAILED(parcel->WriteInt64((AXP::Int64)objectHolder.Get())))
                    return NULL;

                return parcel;
            }
            else {
                parcel = objectHolder->OnTransact(bundle, uri);
                if ((parcel == NULL) && (code == IPC::CommandCode::COMMAND_CREATE))
                    ReleaseObjectHolder(objectHolder, uri, FALSE);

                return parcel;
            }
        }

        AXP::Void OnDeath(IN CONST AXP::Sp<AXP::String> & uri)
        {
            if (mServiceList == NULL)
                return;

            if (uri == NULL)
                return;

            Synchronized(&mServiceList->mLock) {
                AXP::Sp<AXP::List<IPC::CObjectHolder> > list = mServiceList->GetValues();
                if (list == NULL)
                    return;

                Foreach(IPC::CObjectHolder, obj, list) {
                    ReleaseObjectHolder(obj, uri, TRUE);
                }
            }
        }

    private:

        AXP::Sp<IPC::CObjectHolder> CreateOrGetObjectHolder(
            IN CONST AXP::Int32 code,
            IN CONST AXP::Sp<AXP::String> & uri,
            IN CONST AXP::Sp<AXP::CParcel> & bundle,
            OUT AXP::Sp<AXP::CParcel> & parcel)
        {
            if ((mServiceList == NULL) || (bundle == NULL))
                return NULL;

            Synchronized (&mServiceList->mLock) {
                AXP::Sp<IPC::CObjectHolder> obj;
                switch (code) {
                case IPC::CommandCode::COMMAND_CREATE:
                    if (!mServiceList->Empty()) {
                        AXP::Sp<AXP::List<IPC::CObjectHolder> > valueList = mServiceList->GetValues();
                        if (valueList == NULL)
                            return NULL;

                        obj = valueList->Get(0);
                    }
                    else {
                        obj = CreateAObjectHolder();
                        if (obj == NULL)
                            return NULL;

                        if (!mServiceList->InsertUnique((AXP::Int64)obj.Get(), obj))
                            return NULL;
                    }

                    break;
                case IPC::CommandCode::COMMAND_CALLBACK:
                case IPC::CommandCode::COMMAND_CALL:
                case IPC::CommandCode::COMMAND_RELEASE:
                    AXP::Int64 objRef;
                    if (AXP::AFAILED(bundle->ReadInt64(objRef)))
                        return NULL;

                    obj = mServiceList->GetValue(objRef);
                    if (obj == NULL) {
                        parcel = new AXP::CParcel();
                        if (parcel == NULL)
                            return NULL;

                        if (AXP::AFAILED(parcel->WriteInt32((AXP::Int32)IPC::RemoteRefException)))
                            return NULL;

                        return NULL;
                    }

                    break;
                default:
                    return NULL;
                }

                if ((code == IPC::CommandCode::COMMAND_CREATE) || (code == IPC::CommandCode::COMMAND_CALLBACK)) {
                    if (uri != NULL) {
                        IPC::UriManager::StartThread(uri);
                        obj->mUriList->PushBack(uri);
                    }

                    obj->AddRemoteRef();
                }

                return obj;
            }

            return NULL;
        }

        AXP::ARESULT ReleaseObjectHolder(
            IN CONST AXP::Sp<IPC::CObjectHolder> & objectHolder,
            IN CONST AXP::Sp<AXP::String> & uri,
            IN AXP::Boolean delAll)
        {
            if ((mServiceList == NULL) || (objectHolder == NULL))
                return AXP::AE_INVALIDARG;

            Synchronized(&mServiceList->mLock) {
                if (uri == NULL) {
                    if (objectHolder->DecreaseRemoteRef() == 0) {
                        if (!mServiceList->Remove((AXP::Int64)objectHolder.Get()))
                            return AXP::AE_FAIL;
                    }
                }
                else {
                    Foreach(AXP::String, obj, objectHolder->mUriList) {
                        if (uri->Equals(obj)) {
                            objectHolder->mUriList->Detach(obj);
                            if (objectHolder->DecreaseRemoteRef() == 0) {
                                if (!mServiceList->Remove((AXP::Int64)objectHolder.Get()))
                                    return AXP::AE_FAIL;
                            }

                            if (!delAll)
                                break;
                        }
                    }
                }

                return AXP::AS_OK;
            }

            return AXP::AE_FAIL;
        }

    private:

        AXP::Sp<AXP::HashTable<AXP::Int64, IPC::CObjectHolder> > mServiceList;
    };

    STATIC AXP::Boolean RegisterService()
    {
        return AXP::ASUCCEEDED(IPC::ServiceManager::RegisterService(L"TestA.A", new TestA::AStub()));
    }

    STATIC AXP::Boolean sIsRegister = RegisterService();
}
