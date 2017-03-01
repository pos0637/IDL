# IDL
可根据接口描述文件自动生成多语种数据结构代码的生成器(C++/C#/Java/JavaScript ES5/JavaScript ES6/Objective-C)


##项目依赖
- windows环境需要安装Cygwin [http://www.cygwin.com/](http://www.cygwin.com/ "安装Cygwin")
- 在Cygwin模拟器上安装gcc、gdb、make
- 安装完成后，打开Cygwin命令行进入到项目根目录，输入make编译。
- 编译完成后会生成GenerateIDL.exe --help

##IDL语法
支持以下数据类型
Byte
Int8;
Int16
Int32
Int64;
Double;
Boolean;
Int8?;
Int16？;
Int32？；
Int64?;
String;
List<String>;
List<Int8?>;
List<Int32?>;
List<Int64?>;
List<Double?>;
List<Boolean?>；
##生成代码示例：
test.idl文件内容如下：
<pre>
namespace NA
{
    class CBase
    {
        Byte mB;
    }
    
    class CList : CBase
    {
        Int8 a;
        Int16 b;
        Int64 c;
        Double e;
        Boolean bee;
        Int8? f;
        Int64? g;
        String m;
        List<String> lstring;
        List<Int64?> list64;
        List<Double?> listDouble;
    }
        
}
</pre>

1. **生成cpp文件命令：./GenerateIDL.exe  -cpp test.idl**

<pre><code>
#ifndef __TEST_H__
#define __TEST_H__

#include "AXP/cplusplus/xplatform/include/type.h"
#include "AXP/cplusplus/xplatform/include/nullable.h"
#include "AXP/cplusplus/xplatform/include/astring.h"
#include "AXP/cplusplus/xplatform/include/list.h"
#include "AXP/cplusplus/xplatform/include/Parcel.h"
#include "AXP/cplusplus/xplatform/include/parcelable.h"
#include "AXP/cplusplus/libc/include/Common/ClassLoader.h"

namespace NA
{
    class CBase : public AXP::IParcelable, public AXP::CObject
    {
    public:

        AXP::ARESULT WriteToParcel(IN CONST AXP::Sp<AXP::CParcel> & parcel)
        {
            if (parcel == NULL)
                return AXP::AE_FAIL;

            if (AXP::AFAILED(parcel->WriteString(AXP::String::Create(L"NA.CBase"))))
                return AXP::AE_FAIL;

            if (AXP::AFAILED(parcel->WriteByte(mB)))
                return AXP::AE_FAIL;

            return AXP::AS_OK;
        }

        AXP::ARESULT ReadFromParcel(IN CONST AXP::Sp<AXP::CParcel> & parcel)
        {
            if (parcel == NULL)
                return AXP::AE_FAIL;

            AXP::Sp<AXP::String> className;
            if (AXP::AFAILED(parcel->ReadString(className)))
                return AXP::AE_FAIL;

            if (AXP::AFAILED(parcel->ReadByte(mB)))
                return AXP::AE_FAIL;

            return AXP::AS_OK;
        }

        VIRTUAL AXP::Sp<AXP::String> ToString()
        {
            return AXP::String::Create(16, L"%d", mB);
        }

        VIRTUAL AXP::Sp<AXP::String> GetTypeName()
        {
            return AXP::String::Create(L"NA.CBase");
        }

    public:

        STATIC AXP::Sp<AXP::CObject> Create()
        {
            return new CBase();
        }

    public:

        AXP::Int8 mB;
    };

    class CList : public CBase
    {
    public:

        AXP::ARESULT WriteToParcel(IN CONST AXP::Sp<AXP::CParcel> & parcel)
        {
            if (parcel == NULL)
                return AXP::AE_FAIL;

            if (AXP::AFAILED(parcel->WriteString(AXP::String::Create(L"NA.CList"))))
                return AXP::AE_FAIL;

            if (AXP::AFAILED(CBase::WriteToParcel(parcel)))
                return AXP::AE_FAIL;

            if (AXP::AFAILED(parcel->WriteInt8(a)))
                return AXP::AE_FAIL;

            if (AXP::AFAILED(parcel->WriteInt16(b)))
                return AXP::AE_FAIL;

            if (AXP::AFAILED(parcel->WriteInt64(c)))
                return AXP::AE_FAIL;

            if (AXP::AFAILED(parcel->WriteDouble(e)))
                return AXP::AE_FAIL;

            if (AXP::AFAILED(parcel->WriteBoolean(bee)))
                return AXP::AE_FAIL;

            if (AXP::AFAILED(parcel->WriteNullableInt8(f)))
                return AXP::AE_FAIL;

            if (AXP::AFAILED(parcel->WriteNullableInt64(g)))
                return AXP::AE_FAIL;

            if (AXP::AFAILED(parcel->WriteString(m)))
                return AXP::AE_FAIL;

            {
                AXP::Int32 length;
                if (lstring == NULL)
                    length = 0;
                else
                    length = lstring->GetCount();

                AXP::Int8 type = 'L';
                if (AXP::AFAILED(parcel->Write((AXP::PCByte)&type, sizeof(type))))
                    return AXP::AE_FAIL;

                if (AXP::AFAILED(parcel->Write((AXP::PCByte)&length, sizeof(length))))
                    return AXP::AE_FAIL;

                if (lstring) {
                    Foreach(AXP::String, obj, lstring) {
                        if (obj == NULL)
                            return AXP::AE_FAIL;

                        if (AXP::AFAILED(parcel->WriteString(obj)))
                            return AXP::AE_FAIL;
                    }
                }
            }

            {
                AXP::Int32 length;
                if (list64 == NULL)
                    length = 0;
                else
                    length = list64->GetCount();

                AXP::Int8 type = 'L';
                if (AXP::AFAILED(parcel->Write((AXP::PCByte)&type, sizeof(type))))
                    return AXP::AE_FAIL;

                if (AXP::AFAILED(parcel->Write((AXP::PCByte)&length, sizeof(length))))
                    return AXP::AE_FAIL;

                if (list64) {
                    Foreach(AXP::Int64$, obj, list64) {
                        if (obj == NULL)
                            return AXP::AE_FAIL;

                        if (AXP::AFAILED(parcel->WriteNullableInt64(obj->GetValue())))
                            return AXP::AE_FAIL;
                    }
                }
            }

            {
                AXP::Int32 length;
                if (listDouble == NULL)
                    length = 0;
                else
                    length = listDouble->GetCount();

                AXP::Int8 type = 'L';
                if (AXP::AFAILED(parcel->Write((AXP::PCByte)&type, sizeof(type))))
                    return AXP::AE_FAIL;

                if (AXP::AFAILED(parcel->Write((AXP::PCByte)&length, sizeof(length))))
                    return AXP::AE_FAIL;

                if (listDouble) {
                    Foreach(AXP::Double$, obj, listDouble) {
                        if (obj == NULL)
                            return AXP::AE_FAIL;

                        if (AXP::AFAILED(parcel->WriteNullableDouble(obj->GetValue())))
                            return AXP::AE_FAIL;
                    }
                }
            }

            return AXP::AS_OK;
        }

        AXP::ARESULT ReadFromParcel(IN CONST AXP::Sp<AXP::CParcel> & parcel)
        {
            if (parcel == NULL)
                return AXP::AE_FAIL;

            AXP::Sp<AXP::String> className;
            if (AXP::AFAILED(parcel->ReadString(className)))
                return AXP::AE_FAIL;

            if (AXP::AFAILED(CBase::ReadFromParcel(parcel)))
                return AXP::AE_FAIL;

            if (AXP::AFAILED(parcel->ReadInt8(a)))
                return AXP::AE_FAIL;

            if (AXP::AFAILED(parcel->ReadInt16(b)))
                return AXP::AE_FAIL;

            if (AXP::AFAILED(parcel->ReadInt64(c)))
                return AXP::AE_FAIL;

            if (AXP::AFAILED(parcel->ReadDouble(e)))
                return AXP::AE_FAIL;

            if (AXP::AFAILED(parcel->ReadBoolean(bee)))
                return AXP::AE_FAIL;

            if (AXP::AFAILED(parcel->ReadNullableInt8(f)))
                return AXP::AE_FAIL;

            if (AXP::AFAILED(parcel->ReadNullableInt64(g)))
                return AXP::AE_FAIL;

            if (AXP::AFAILED(parcel->ReadString(m)))
                return AXP::AE_FAIL;

            {
                AXP::Int8 type;
                if (AXP::AFAILED(parcel->Read((AXP::PByte)&type, sizeof(type), sizeof(type))))
                    return AXP::AE_FAIL;

                if (type != 'L')
                    return AXP::AE_FAIL;

                AXP::Int32 length;
                if (AXP::AFAILED(parcel->Read((AXP::PByte)&length, sizeof(length), sizeof(length))))
                    return AXP::AE_FAIL;

                lstring = new AXP::List<AXP::String>();
                if (lstring == NULL)
                    return AXP::AE_FAIL;

                for (AXP::Int32 i = 0; i < length; i++) {
                    AXP::Sp<AXP::String> obj;
                    if (AXP::AFAILED(parcel->ReadString(obj)))
                        return AXP::AE_FAIL;

                    if (!lstring->PushBack(obj))
                        return AXP::AE_FAIL;
                }
            }

            {
                AXP::Int8 type;
                if (AXP::AFAILED(parcel->Read((AXP::PByte)&type, sizeof(type), sizeof(type))))
                    return AXP::AE_FAIL;

                if (type != 'L')
                    return AXP::AE_FAIL;

                AXP::Int32 length;
                if (AXP::AFAILED(parcel->Read((AXP::PByte)&length, sizeof(length), sizeof(length))))
                    return AXP::AE_FAIL;

                list64 = new AXP::List<AXP::Int64$>();
                if (list64 == NULL)
                    return AXP::AE_FAIL;

                for (AXP::Int32 i = 0; i < length; i++) {
                    AXP::Int64$ obj;
                    if (AXP::AFAILED(parcel->ReadNullableInt64(obj)))
                        return AXP::AE_FAIL;

                    if (!list64->PushBack(new AXP::Int64$(obj)))
                        return AXP::AE_FAIL;
                }
            }

            {
                AXP::Int8 type;
                if (AXP::AFAILED(parcel->Read((AXP::PByte)&type, sizeof(type), sizeof(type))))
                    return AXP::AE_FAIL;

                if (type != 'L')
                    return AXP::AE_FAIL;

                AXP::Int32 length;
                if (AXP::AFAILED(parcel->Read((AXP::PByte)&length, sizeof(length), sizeof(length))))
                    return AXP::AE_FAIL;

                listDouble = new AXP::List<AXP::Double$>();
                if (listDouble == NULL)
                    return AXP::AE_FAIL;

                for (AXP::Int32 i = 0; i < length; i++) {
                    AXP::Double$ obj;
                    if (AXP::AFAILED(parcel->ReadNullableDouble(obj)))
                        return AXP::AE_FAIL;

                    if (!listDouble->PushBack(new AXP::Double$(obj)))
                        return AXP::AE_FAIL;
                }
            }

            return AXP::AS_OK;
        }

        VIRTUAL AXP::Sp<AXP::String> ToString()
        {
            AXP::Sp<AXP::String> json = AXP::String::Create(L"{");
            if (json == NULL)
                return NULL;

            json = AXP::String::Create(json->Length() + 47, L"%ls\"a\":\"%d\",", (AXP::PCWStr)*json, a);
            if (json == NULL)
                return NULL;

            json = AXP::String::Create(json->Length() + 47, L"%ls\"b\":\"%d\",", (AXP::PCWStr)*json, b);
            if (json == NULL)
                return NULL;

            json = AXP::String::Create(json->Length() + 47, L"%ls\"c\":\"%lld\",", (AXP::PCWStr)*json, c);
            if (json == NULL)
                return NULL;

            json = AXP::String::Create(json->Length() + 315, L"%ls\"e\":\"%.2f\",", (AXP::PCWStr)*json, e);
            if (json == NULL)
                return NULL;

            json = AXP::String::Create(json->Length() + 47, L"%ls\"bee\":\"%ls\",", (AXP::PCWStr)*json, bee ? L"true" : L"false");
            if (json == NULL)
                return NULL;

            if (!f.HasValue()) {
                json = AXP::String::Create(json->Length() + 40, L"%ls\"f\":\"\",", (AXP::PCWStr)*json);
                if (json == NULL)
                    return NULL;
            }
            else {
                json = AXP::String::Create(json->Length() + 64, L"%ls\"f\":\"%d\",", (AXP::PCWStr)*json, f.GetValue());
                if (json == NULL)
                    return NULL;
            }

            if (!g.HasValue()) {
                json = AXP::String::Create(json->Length() + 40, L"%ls\"g\":\"\",", (AXP::PCWStr)*json);
                if (json == NULL)
                    return NULL;
            }
            else {
                json = AXP::String::Create(json->Length() + 64, L"%ls\"g\":\"%lld\",", (AXP::PCWStr)*json, g.GetValue());
                if (json == NULL)
                    return NULL;
            }

            if (m == NULL) {
                json = AXP::String::Create(json->Length() + 40, L"%ls\"m\":\"\",", (AXP::PCWStr)*json);
                if (json == NULL)
                    return NULL;
            }
            else {
                json = AXP::String::Create(json->Length() + m->Length() + 40, L"%ls\"m\":\"%ls\",", (AXP::PCWStr)*json, (AXP::PCWStr)*m);
                if (json == NULL)
                    return NULL;
            }

            if (lstring == NULL) {
                json = AXP::String::Create(json->Length() + 40, L"%ls\"lstring\":[],", (AXP::PCWStr)*json);
                if (json == NULL)
                    return NULL;
            }
            else {
                AXP::Sp<AXP::String> jsonTmp = AXP::String::Create(L"\"lstring\":[");
                if (jsonTmp == NULL)
                    return NULL;

                for(AXP::Int32 i = 0; i < lstring->GetCount(); ++i) {
                    AXP::Sp<AXP::String> obj = (*lstring)[i];
                    AXP::PCWStr comma;
                    if (i < lstring->GetCount() - 1)
                        comma = L",";
                    else
                        comma = L"";

                    if (obj == NULL)
                        jsonTmp = AXP::String::Create(json->Length() + 39, L"%ls\"\"%ls", (AXP::PCWStr)*json, comma);
                    else
                        jsonTmp = AXP::String::Create(jsonTmp->Length() + obj->Length() + 39, L"%ls\"%ls\"%ls", (AXP::PCWStr)*jsonTmp, i, (AXP::PCWStr)*obj, comma);
                    if (jsonTmp == NULL)
                        return NULL;
                }

                jsonTmp = AXP::String::Create(jsonTmp->Length() + 7, L"%ls],", (AXP::PCWStr)*jsonTmp);
                if (jsonTmp == NULL)
                    return NULL;

                json = AXP::String::Create(json->Length() + jsonTmp->Length() + 2, L"%ls%ls", (AXP::PCWStr)*json, (AXP::PCWStr)*jsonTmp);
                if (json == NULL)
                    return NULL;
            }

            if (list64 == NULL) {
                json = AXP::String::Create(json->Length() + 40, L"%ls\"list64\":[],", (AXP::PCWStr)*json);
                if (json == NULL)
                    return NULL;
            }
            else {
                AXP::Sp<AXP::String> jsonTmp = AXP::String::Create(L"\"list64\":[");
                if (jsonTmp == NULL)
                    return NULL;

                for(AXP::Int32 i = 0; i < list64->GetCount(); ++i) {
                    AXP::Sp<AXP::Int64$> obj = (*list64)[i];
                    AXP::PCWStr comma;
                    if (i < list64->GetCount() - 1)
                        comma = L",";
                    else
                        comma = L"";

                    if ((obj == NULL) || (!obj->HasValue()))
                        jsonTmp = AXP::String::Create(jsonTmp->Length() + 47, L"%ls\"\"%ls", (AXP::PCWStr)*jsonTmp, comma);
                    else
                        jsonTmp = AXP::String::Create(jsonTmp->Length() + 47, L"%ls\"%lld\"%ls", (AXP::PCWStr)*jsonTmp, obj->GetValue(), comma);

                    if (jsonTmp == NULL)
                        return NULL;
                }

                jsonTmp = AXP::String::Create(jsonTmp->Length() + 7, L"%ls],", (AXP::PCWStr)*jsonTmp);
                if (jsonTmp == NULL)
                    return NULL;

                json = AXP::String::Create(json->Length() + jsonTmp->Length() + 2, L"%ls%ls", (AXP::PCWStr)*json, (AXP::PCWStr)*jsonTmp);
                if (json == NULL)
                    return NULL;
            }

            if (listDouble == NULL) {
                json = AXP::String::Create(json->Length() + 40, L"%ls\"listDouble\":[]", (AXP::PCWStr)*json);
                if (json == NULL)
                    return NULL;
            }
            else {
                AXP::Sp<AXP::String> jsonTmp = AXP::String::Create(L"\"listDouble\":[");
                if (jsonTmp == NULL)
                    return NULL;

                for(AXP::Int32 i = 0; i < listDouble->GetCount(); ++i) {
                    AXP::Sp<AXP::Double$> obj = (*listDouble)[i];
                    AXP::PCWStr comma;
                    if (i < listDouble->GetCount() - 1)
                        comma = L",";
                    else
                        comma = L"";

                    if ((obj == NULL) || (!obj->HasValue()))
                        jsonTmp = AXP::String::Create(jsonTmp->Length() + 47, L"%ls\"\"%ls", (AXP::PCWStr)*jsonTmp, obj->GetValue(), comma);
                    else
                        jsonTmp = AXP::String::Create(jsonTmp->Length() + 47, L"%ls\"%.2f\"%ls", (AXP::PCWStr)*jsonTmp, obj->GetValue(), comma);

                    if (jsonTmp == NULL)
                        return NULL;
                }

                jsonTmp = AXP::String::Create(jsonTmp->Length() + 7, L"%ls]", (AXP::PCWStr)*jsonTmp);
                if (jsonTmp == NULL)
                    return NULL;

                json = AXP::String::Create(json->Length() + jsonTmp->Length() + 2, L"%ls%ls", (AXP::PCWStr)*json, (AXP::PCWStr)*jsonTmp);
                if (json == NULL)
                    return NULL;
            }

            return AXP::String::Create(json->Length() + 7, L"%ls}", (AXP::PCWStr)*json);
        }

        VIRTUAL AXP::Sp<AXP::String> GetTypeName()
        {
            return AXP::String::Create(L"NA.CList");
        }

    public:

        STATIC AXP::Sp<AXP::CObject> Create()
        {
            return new CList();
        }

    public:

        AXP::Int8 a;
        AXP::Int16 b;
        AXP::Int64 c;
        AXP::Double e;
        AXP::Boolean bee;
        AXP::Int8$ f;
        AXP::Int64$ g;
        AXP::Sp<AXP::String> m;
        AXP::Sp<AXP::List<AXP::String > > lstring;
        AXP::Sp<AXP::List<AXP::Int64$ > > list64;
        AXP::Sp<AXP::List<AXP::Double$ > > listDouble;
    };

    STATIC AXP::Boolean _NA_CBase_ = AXP::Libc::Common::ClassLoader::RegisterClassCreator(L"NA.CBase", NA::CBase::Create);
    STATIC AXP::Boolean _NA_CList_ = AXP::Libc::Common::ClassLoader::RegisterClassCreator(L"NA.CList", NA::CList::Create);
}
#endif // __TEST_H__
</code>
</pre>
2. **生成csharp文件命令：./GenerateIDL.exe  -csharp test.idl**
<pre><code>
using System;
using System.Collections.Generic;
using AXP;

namespace NA
{
    public class CBase : AXP.IParcelable
    {
        public SByte mB;

        public virtual Int32 WriteToParcel(Parcel parcel)
        {
            if (parcel == null)
                return AResult.AE_FAIL;

            if (AResult.AFAILED(parcel.WriteString("NA.CBase")))
                return AResult.AE_FAIL;

            if (AResult.AFAILED(parcel.WriteByte(mB)))
                return AResult.AE_FAIL;

            return AResult.AS_OK;
        }

        public virtual Int32 ReadFromParcel(Parcel parcel)
        {
            if (parcel == null)
                return AResult.AE_FAIL;

            String description = null;
            if (AResult.AFAILED(parcel.ReadString(ref description)))
                return AResult.AE_FAIL;

            if (AResult.AFAILED(parcel.ReadByte(ref mB)))
                return AResult.AE_FAIL;

            return AResult.AS_OK;
        }

        public override String ToString()
        {
            return String.Format("{0}", mB);
        }

        public virtual String GetTypeName()
        {
            return "NA.CBase";
        }

        public static IParcelable Create()
        {
            return new CBase();
        }
    };

    public class CList : CBase
    {
        public SByte a;

        public Int16 b;

        public Int64 c;

        public Double e;

        public Boolean bee;

        public SByte? f;

        public Int64? g;

        public String m;

        public List<String> lstring = new List<String>();

        public List<Int64?> list64 = new List<Int64?>();

        public List<Double?> listDouble = new List<Double?>();

        public override Int32 WriteToParcel(Parcel parcel)
        {
            if (parcel == null)
                return AResult.AE_FAIL;

            if (AResult.AFAILED(parcel.WriteString("NA.CList")))
                return AResult.AE_FAIL;

            if (AResult.AFAILED(base.WriteToParcel(parcel)))
                return AResult.AE_FAIL;

            if (AResult.AFAILED(parcel.WriteInt8(a)))
                return AResult.AE_FAIL;

            if (AResult.AFAILED(parcel.WriteInt16(b)))
                return AResult.AE_FAIL;

            if (AResult.AFAILED(parcel.WriteInt64(c)))
                return AResult.AE_FAIL;

            if (AResult.AFAILED(parcel.WriteDouble(e)))
                return AResult.AE_FAIL;

            if (AResult.AFAILED(parcel.WriteBoolean(bee)))
                return AResult.AE_FAIL;

            if (AResult.AFAILED(parcel.WriteNullableInt8(f)))
                return AResult.AE_FAIL;

            if (AResult.AFAILED(parcel.WriteNullableInt64(g)))
                return AResult.AE_FAIL;

            if (AResult.AFAILED(parcel.WriteString(m)))
                return AResult.AE_FAIL;

            {
                Int32 length = 0;
                if (lstring == null)
                    length = 0;
                else
                    length = lstring.Count;

                string typeStr = "L";
                Byte[] type = System.Text.Encoding.ASCII.GetBytes(typeStr);
                if (AResult.AFAILED(parcel.Write(type)))
                    return AResult.AE_FAIL;

                Byte[] lengthArray = System.BitConverter.GetBytes(length);
                if (AResult.AFAILED(parcel.Write(lengthArray)))
                    return AResult.AE_FAIL;

                if (lstring != null) {
                    foreach(String obj in lstring) {
                        if (obj == null)
                            return AResult.AE_FAIL;

                        if (AResult.AFAILED(parcel.WriteString(obj)))
                            return AResult.AE_FAIL;
                    }
                }
            }

            {
                Int32 length = 0;
                if (list64 == null)
                    length = 0;
                else
                    length = list64.Count;

                string typeStr = "L";
                Byte[] type = System.Text.Encoding.ASCII.GetBytes(typeStr);
                if (AResult.AFAILED(parcel.Write(type)))
                    return AResult.AE_FAIL;

                Byte[] lengthArray = System.BitConverter.GetBytes(length);
                if (AResult.AFAILED(parcel.Write(lengthArray)))
                    return AResult.AE_FAIL;

                if (list64 != null) {
                    foreach(Int64? obj in list64) {
                        if (obj == null)
                            return AResult.AE_FAIL;

                        if (AResult.AFAILED(parcel.WriteNullableInt64(obj)))
                            return AResult.AE_FAIL;
                    }
                }
            }

            {
                Int32 length = 0;
                if (listDouble == null)
                    length = 0;
                else
                    length = listDouble.Count;

                string typeStr = "L";
                Byte[] type = System.Text.Encoding.ASCII.GetBytes(typeStr);
                if (AResult.AFAILED(parcel.Write(type)))
                    return AResult.AE_FAIL;

                Byte[] lengthArray = System.BitConverter.GetBytes(length);
                if (AResult.AFAILED(parcel.Write(lengthArray)))
                    return AResult.AE_FAIL;

                if (listDouble != null) {
                    foreach(Double? obj in listDouble) {
                        if (obj == null)
                            return AResult.AE_FAIL;

                        if (AResult.AFAILED(parcel.WriteNullableDouble(obj)))
                            return AResult.AE_FAIL;
                    }
                }
            }

            return AResult.AS_OK;
        }

        public override Int32 ReadFromParcel(Parcel parcel)
        {
            if (parcel == null)
                return AResult.AE_FAIL;

            String description = null;
            if (AResult.AFAILED(parcel.ReadString(ref description)))
                return AResult.AE_FAIL;

            if (AResult.AFAILED(base.ReadFromParcel(parcel)))
                return AResult.AE_FAIL;

            if (AResult.AFAILED(parcel.ReadInt8(ref a)))
                return AResult.AE_FAIL;

            if (AResult.AFAILED(parcel.ReadInt16(ref b)))
                return AResult.AE_FAIL;

            if (AResult.AFAILED(parcel.ReadInt64(ref c)))
                return AResult.AE_FAIL;

            if (AResult.AFAILED(parcel.ReadDouble(ref e)))
                return AResult.AE_FAIL;

            if (AResult.AFAILED(parcel.ReadBoolean(ref bee)))
                return AResult.AE_FAIL;

            if (AResult.AFAILED(parcel.ReadNullableInt8(ref f)))
                return AResult.AE_FAIL;

            if (AResult.AFAILED(parcel.ReadNullableInt64(ref g)))
                return AResult.AE_FAIL;

            if (AResult.AFAILED(parcel.ReadString(ref m)))
                return AResult.AE_FAIL;

            {
                lstring = new List<String>();
                if (lstring == null)
                    return AResult.AE_FAIL;

                Byte[] type = null;
                if (AResult.AFAILED(parcel.Read(ref type, 1)))
                    return AResult.AE_FAIL;

                String typeStr = System.Text.Encoding.ASCII.GetString(type);
                if (typeStr != "L")
                    return AResult.AE_FAIL;

                Byte[] lengthArray = null;
                if (AResult.AFAILED(parcel.Read(ref lengthArray, 4)))
                    return AResult.AE_FAIL;

                Int32 length = System.BitConverter.ToInt32(lengthArray, 0);
                for (Int32 i = 0; i < length; i++) {
                    String obj = null;
                    if (AResult.AFAILED(parcel.ReadString(ref obj)))
                        return AResult.AE_FAIL;

                    lstring.Add(obj);
                }
            }

            {
                list64 = new List<Int64?>();
                if (list64 == null)
                    return AResult.AE_FAIL;

                Byte[] type = null;
                if (AResult.AFAILED(parcel.Read(ref type, 1)))
                    return AResult.AE_FAIL;

                String typeStr = System.Text.Encoding.ASCII.GetString(type);
                if (typeStr != "L")
                    return AResult.AE_FAIL;

                Byte[] lengthArray = null;
                if (AResult.AFAILED(parcel.Read(ref lengthArray, 4)))
                    return AResult.AE_FAIL;

                Int32 length = System.BitConverter.ToInt32(lengthArray, 0);
                for (Int32 i = 0; i < length; i++) {
                    Int64? obj = null;
                    if (AResult.AFAILED(parcel.ReadNullableInt64(ref obj)))
                        return AResult.AE_FAIL;

                    list64.Add(obj);
                }
            }

            {
                listDouble = new List<Double?>();
                if (listDouble == null)
                    return AResult.AE_FAIL;

                Byte[] type = null;
                if (AResult.AFAILED(parcel.Read(ref type, 1)))
                    return AResult.AE_FAIL;

                String typeStr = System.Text.Encoding.ASCII.GetString(type);
                if (typeStr != "L")
                    return AResult.AE_FAIL;

                Byte[] lengthArray = null;
                if (AResult.AFAILED(parcel.Read(ref lengthArray, 4)))
                    return AResult.AE_FAIL;

                Int32 length = System.BitConverter.ToInt32(lengthArray, 0);
                for (Int32 i = 0; i < length; i++) {
                    Double? obj = null;
                    if (AResult.AFAILED(parcel.ReadNullableDouble(ref obj)))
                        return AResult.AE_FAIL;

                    listDouble.Add(obj);
                }
            }

            return AResult.AS_OK;
        }

        public override String ToString()
        {
            String json = "{";
            if (json == null)
                return null;

            json = String.Format("{0}\"a\":\"{1}\",", json, a);
            if (json == null)
                return null;

            json = String.Format("{0}\"b\":\"{1}\",", json, b);
            if (json == null)
                return null;

            json = String.Format("{0}\"c\":\"{1}\",", json, c);
            if (json == null)
                return null;

            json = String.Format("{0}\"e\":\"{1}\",", json, e);
            if (json == null)
                return null;

            json = String.Format("{0}\"bee\":\"{1}\",", json, bee);
            if (json == null)
                return null;

            if (f == null) {
                json = String.Format("{0}\"f\":\"\",", json);
                if (json == null)
                    return null;
            }
            else {
                json = String.Format("{0}\"f\":\"{1}\",", json, f);
                if (json == null)
                    return null;
            }

            if (g == null) {
                json = String.Format("{0}\"g\":\"\",", json);
                if (json == null)
                    return null;
            }
            else {
                json = String.Format("{0}\"g\":\"{1}\",", json, g);
                if (json == null)
                    return null;
            }

            if (m == null) {
                json = String.Format("{0}\"m\":\"\",", json);
                if (json == null)
                    return null;
            }
            else {
                json = String.Format("{0}\"m\":\"{1}\",", json, m);
                if (json == null)
                    return null;
            }

            if (lstring == null) {
                json = String.Format("{0}\"lstring\":[],", json);
                if (json == null)
                    return null;
            }
            else {
                String jsonTmp = String.Format("\"lstring\":[");
                if (jsonTmp == null)
                    return null;

                for(Int32 i = 0; i < lstring.Count; ++i) {
                    String obj = lstring[i];
                    String comma = null;
                    if (i < lstring.Count - 1)
                        comma = ",";
                    else
                        comma = "";

                    jsonTmp = String.Format("{0}\"{1}\"{2}", jsonTmp, obj, comma);
                    if (jsonTmp == null)
                        return null;
                }

                jsonTmp = String.Format("{0}],", jsonTmp);
                if (jsonTmp == null)
                    return null;

                json = String.Format("{0}{1}", json, jsonTmp);
                if (json == null)
                    return null;
            }

            if (list64 == null) {
                json = String.Format("{0}\"list64\":[],", json);
                if (json == null)
                    return null;
            }
            else {
                String jsonTmp = String.Format("\"list64\":[");
                if (jsonTmp == null)
                    return null;

                for(Int32 i = 0; i < list64.Count; ++i) {
                    Int64? obj = list64[i];
                    String comma = null;
                    if (i < list64.Count - 1)
                        comma = ",";
                    else
                        comma = "";

                    jsonTmp = String.Format("{0}\"{1}\"{2}", jsonTmp, obj, comma);
                    if (jsonTmp == null)
                        return null;
                }

                jsonTmp = String.Format("{0}],", jsonTmp);
                if (jsonTmp == null)
                    return null;

                json = String.Format("{0}{1}", json, jsonTmp);
                if (json == null)
                    return null;
            }

            if (listDouble == null) {
                json = String.Format("{0}\"listDouble\":[]", json);
                if (json == null)
                    return null;
            }
            else {
                String jsonTmp = String.Format("\"listDouble\":[");
                if (jsonTmp == null)
                    return null;

                for(Int32 i = 0; i < listDouble.Count; ++i) {
                    Double? obj = listDouble[i];
                    String comma = null;
                    if (i < listDouble.Count - 1)
                        comma = ",";
                    else
                        comma = "";

                    jsonTmp = String.Format("{0}\"{1}\"{2}", jsonTmp, obj, comma);
                    if (jsonTmp == null)
                        return null;
                }

                jsonTmp = String.Format("{0}]", jsonTmp);
                if (jsonTmp == null)
                    return null;

                json = String.Format("{0}{1}", json, jsonTmp);
                if (json == null)
                    return null;
            }

            return String.Format("{0}}}", json);
        }

        public override String GetTypeName()
        {
            return "NA.CList";
        }

        public static new IParcelable Create()
        {
            return new CList();
        }
    };
}
</code></pre>

3. **生成objective-c文件命令：./GenerateIDL.exe  -objc test.idl**
<pre><code>
#import "test.h"
#import "AXP/objective-c/libc/include/Common/ClassLoader.h"

@implementation CBase

@synthesize mB;

- (AXP::ARESULT)WriteToParcel: (CParcel*)parcel
{
    if (parcel == nil)
        return AXP::AE_FAIL;

    if (AXP::AFAILED([parcel WriteString:@"NA.CBase"]))
        return AXP::AE_FAIL;

    if (AXP::AFAILED([parcel WriteByte: mB]))
        return AXP::AE_FAIL;

    return AXP::AS_OK;
}

- (AXP::ARESULT)ReadFromParcel: (CParcel*)parcel
{
    if (parcel == nil)
        return AXP::AE_FAIL;

    @try {
        [parcel ReadString];
    }
    @catch (NSException * exception) {
        return AXP::AE_FAIL;
    }

    if (AXP::AFAILED([parcel ReadByte: &mB]))
        return AXP::AE_FAIL;

    return AXP::AS_OK;
}

- (void)Reset
{
}

- (NSString*)GetTypeName
{
    return @"NA.CBase";
}

@end

@implementation CList

@synthesize a;
@synthesize b;
@synthesize c;
@synthesize e;
@synthesize bee;
@synthesize f;
@synthesize g;
@synthesize m;
@synthesize lstring;
@synthesize list64;
@synthesize listDouble;

- (AXP::ARESULT)WriteToParcel: (CParcel*)parcel
{
    if (parcel == nil)
        return AXP::AE_FAIL;

    if (AXP::AFAILED([parcel WriteString:@"NA.CList"]))
        return AXP::AE_FAIL;

    if (AXP::AFAILED([super WriteToParcel:parcel]))
        return AXP::AE_FAIL;

    if (AXP::AFAILED([parcel WriteInt8: a]))
        return AXP::AE_FAIL;

    if (AXP::AFAILED([parcel WriteInt16: b]))
        return AXP::AE_FAIL;

    if (AXP::AFAILED([parcel WriteInt64: c]))
        return AXP::AE_FAIL;

    if (AXP::AFAILED([parcel WriteDouble: e]))
        return AXP::AE_FAIL;

    if (AXP::AFAILED([parcel WriteBoolean: bee]))
        return AXP::AE_FAIL;

    if (AXP::AFAILED([parcel WriteNullableInt8: f]))
        return AXP::AE_FAIL;

    if (AXP::AFAILED([parcel WriteNullableInt64: g]))
        return AXP::AE_FAIL;

    if (AXP::AFAILED([parcel WriteString: m]))
        return AXP::AE_FAIL;

    {
        int32_t length = 0;
        if (lstring == nil)
            length = 0;
        else
            length = (int32_t)lstring.count;

        AXP::Char type = 'L';
        if (AXP::AFAILED([parcel Write:(Byte*)&type Length: sizeof(type)]))
            return AXP::AE_FAIL;

        if (AXP::AFAILED([parcel Write:(Byte*)&length Length: sizeof(length)]))
            return AXP::AE_FAIL;

        if (lstring) {
            for (NSString * obj in lstring) {
                if (obj == nil)
                    return AXP::AE_FAIL;

                if (AXP::AFAILED([parcel WriteString: obj]))
                    return AXP::AE_FAIL;
            }
        }
    }

    {
        int32_t length = 0;
        if (list64 == nil)
            length = 0;
        else
            length = (int32_t)list64.count;

        AXP::Char type = 'L';
        if (AXP::AFAILED([parcel Write:(Byte*)&type Length: sizeof(type)]))
            return AXP::AE_FAIL;

        if (AXP::AFAILED([parcel Write:(Byte*)&length Length: sizeof(length)]))
            return AXP::AE_FAIL;

        if (list64) {
            for (NSNumber * obj in list64) {
                if (obj == nil)
                    return AXP::AE_FAIL;

                if (AXP::AFAILED([parcel WriteNullableInt64: obj]))
                    return AXP::AE_FAIL;
            }
        }
    }

    {
        int32_t length = 0;
        if (listDouble == nil)
            length = 0;
        else
            length = (int32_t)listDouble.count;

        AXP::Char type = 'L';
        if (AXP::AFAILED([parcel Write:(Byte*)&type Length: sizeof(type)]))
            return AXP::AE_FAIL;

        if (AXP::AFAILED([parcel Write:(Byte*)&length Length: sizeof(length)]))
            return AXP::AE_FAIL;

        if (listDouble) {
            for (NSNumber * obj in listDouble) {
                if (obj == nil)
                    return AXP::AE_FAIL;

                if (AXP::AFAILED([parcel WriteNullableDouble: obj]))
                    return AXP::AE_FAIL;
            }
        }
    }

    return AXP::AS_OK;
}

- (AXP::ARESULT)ReadFromParcel: (CParcel*)parcel
{
    if (parcel == nil)
        return AXP::AE_FAIL;

    @try {
        [parcel ReadString];
    }
    @catch (NSException * exception) {
        return AXP::AE_FAIL;
    }

    if (AXP::AFAILED([super ReadFromParcel:parcel]))
        return AXP::AE_FAIL;

    if (AXP::AFAILED([parcel ReadInt8: &a]))
        return AXP::AE_FAIL;

    if (AXP::AFAILED([parcel ReadInt16: &b]))
        return AXP::AE_FAIL;

    if (AXP::AFAILED([parcel ReadInt64: &c]))
        return AXP::AE_FAIL;

    if (AXP::AFAILED([parcel ReadDouble: &e]))
        return AXP::AE_FAIL;

    if (AXP::AFAILED([parcel ReadBoolean: &bee]))
        return AXP::AE_FAIL;

    @try {
        f = [parcel ReadNullableInt8];
    }
    @catch (NSException * exception) {
        return AXP::AE_FAIL;
    }

    @try {
        g = [parcel ReadNullableInt64];
    }
    @catch (NSException * exception) {
        return AXP::AE_FAIL;
    }

    @try {
        m = [parcel ReadString];
    }
    @catch (NSException * exception) {
        return AXP::AE_FAIL;
    }

    {
        AXP::Char type;
        if (AXP::AFAILED([parcel Read: (Byte*)&type DstLength: sizeof(type) Length: sizeof(type)]))
            return AXP::AE_FAIL;

        if (type != 'L')
            return AXP::AE_FAIL;

        int32_t length;
        if (AXP::AFAILED([parcel Read: (Byte*)&length DstLength: sizeof(length) Length: sizeof(length)]))
            return AXP::AE_FAIL;

        lstring = [[NSMutableArray alloc] init];
        if (lstring == nil)
            return AXP::AE_FAIL;

        for (int32_t i = 0; i < length; i++) {
            @try {
                NSString * obj = [parcel ReadString];
                [lstring addObject: obj];
            }
            @catch (NSException * exception) {
                return AXP::AE_FAIL;
            }
        }
    }

    {
        AXP::Char type;
        if (AXP::AFAILED([parcel Read: (Byte*)&type DstLength: sizeof(type) Length: sizeof(type)]))
            return AXP::AE_FAIL;

        if (type != 'L')
            return AXP::AE_FAIL;

        int32_t length;
        if (AXP::AFAILED([parcel Read: (Byte*)&length DstLength: sizeof(length) Length: sizeof(length)]))
            return AXP::AE_FAIL;

        list64 = [[NSMutableArray alloc] init];
        if (list64 == nil)
            return AXP::AE_FAIL;

        for (int32_t i = 0; i < length; i++) {
            @try {
                NSNumber * obj = [parcel ReadNullableInt64];
                [list64 addObject: obj];
            }
            @catch (NSException * exception) {
                return AXP::AE_FAIL;
            }
        }
    }

    {
        AXP::Char type;
        if (AXP::AFAILED([parcel Read: (Byte*)&type DstLength: sizeof(type) Length: sizeof(type)]))
            return AXP::AE_FAIL;

        if (type != 'L')
            return AXP::AE_FAIL;

        int32_t length;
        if (AXP::AFAILED([parcel Read: (Byte*)&length DstLength: sizeof(length) Length: sizeof(length)]))
            return AXP::AE_FAIL;

        listDouble = [[NSMutableArray alloc] init];
        if (listDouble == nil)
            return AXP::AE_FAIL;

        for (int32_t i = 0; i < length; i++) {
            @try {
                NSNumber * obj = [parcel ReadNullableDouble];
                [listDouble addObject: obj];
            }
            @catch (NSException * exception) {
                return AXP::AE_FAIL;
            }
        }
    }

    return AXP::AS_OK;
}

- (void)Reset
{
    [super Reset];

    a = INT8_MIN;
    b = INT16_MIN;
    c = INT64_MIN;
    bee = NO;
    f = nil;
    g = nil;
    m = nil;
    lstring = nil;
    list64 = nil;
    listDouble = nil;
}

- (NSString*)GetTypeName
{
    return @"NA.CList";
}

@end

STATIC id Create_NA_CBase()
{
    return [[CBase alloc] init];
}

STATIC AXP::Boolean __NA_CBase__ = RegisterClassCreator(L"NA.CBase", Create_NA_CBase);

STATIC id Create_NA_CList()
{
    return [[CList alloc] init];
}

STATIC AXP::Boolean __NA_CList__ = RegisterClassCreator(L"NA.CList", Create_NA_CList);
</code></pre>
4. **生成javascript文件命令：./GenerateIDL.exe  -javascript test.idl**
<pre><code>
define(["core/parcel", "core/parcelable"], function ()
{
    $.declareClass("NA.CBase", XspWeb.Core.Parcelable, {
        ctor: function ()
        {
            this.mB = 0;
        },

        writeToParcel: function (parcel)
        {
            if (parcel == null)
                return;

            parcel.writeString("NA.CBase");
            parcel.writeByte(this.mB);
        },

        readFromParcel: function (parcel)
        {
            if (parcel == null)
                return;

            parcel.readString();
            this.mB = parcel.readByte();
        }
    });

    $.declareClass("NA.CList", NA.CBase, {
        ctor: function ()
        {
            this.parent();
            this.a = 0;
            this.b = 0;
            this.c = 0;
            this.e = 0;
            this.bee = true;
            this.f = null;
            this.g = null;
            this.m = null;
            this.lstring = [];
            this.list64 = [];
            this.listDouble = [];
        },

        writeToParcel: function (parcel)
        {
            if (parcel == null)
                return;

            parcel.writeString("NA.CList");
            this.parent(parcel);
            parcel.writeInt8(this.a);
            parcel.writeInt16(this.b);
            parcel.writeInt64(this.c);
            parcel.writeDouble(this.e);
            parcel.writeBoolean(this.bee);
            parcel.writeNullableInt8(this.f);
            parcel.writeNullableInt64(this.g);
            parcel.writeString(this.m);
            if (this.lstring) {
                parcel.write("L", this.lstring.length);
                for (var i = 0; i < this.lstring.length; i++) {
                    if (this.lstring[i])
                        parcel.writeString(this.lstring[i]);
                }
            }
            else
                parcel.write("L", 0);

            if (this.list64) {
                parcel.write("L", this.list64.length);
                for (var i = 0; i < this.list64.length; i++) {
                    if (this.list64[i])
                        parcel.writeNullableInt64(this.list64[i]);
                }
            }
            else
                parcel.write("L", 0);

            if (this.listDouble) {
                parcel.write("L", this.listDouble.length);
                for (var i = 0; i < this.listDouble.length; i++) {
                    if (this.listDouble[i])
                        parcel.writeNullableDouble(this.listDouble[i]);
                }
            }
            else
                parcel.write("L", 0);

        },

        readFromParcel: function (parcel)
        {
            if (parcel == null)
                return;

            parcel.readString();
            this.parent(parcel);
            this.a = parcel.readInt8();
            this.b = parcel.readInt16();
            this.c = parcel.readInt64();
            this.e = parcel.readDouble();
            this.bee = parcel.readBoolean();
            this.f = parcel.readNullableInt8();
            this.g = parcel.readNullableInt64();
            this.m = parcel.readString();
            var list = [];
            var length = parcel.read();
            for (var i = 0; i < length; i++) {
                var obj = parcel.readString();
                list.push(obj);
            }
            this.lstring = list;
            var list = [];
            var length = parcel.read();
            for (var i = 0; i < length; i++) {
                var obj = parcel.readNullableInt64();
                list.push(obj);
            }
            this.list64 = list;
            var list = [];
            var length = parcel.read();
            for (var i = 0; i < length; i++) {
                var obj = parcel.readNullableDouble();
                list.push(obj);
            }
            this.listDouble = list;
        }
    });
});
</code></pre>
5. **生成es6文件命令：./GenerateIDL.exe  -es6 test.idl**
<pre><code>
"use strict";

import Parcelabel from "axp/parcelable";
import Parcel from "axp/parcel";
import ServiceConnection from "axp/serviceConnection";
import * as IPC from "axp/ipc";
import ClassLoader from "../classloader";

export class CBase extends Parcelable
{
    constructor()
    {
        this.mB = 0;
    }

    writeToParcel(parcel)
    {
        if (parcel == null)
            return;

        parcel.writeString("NA.CBase");

        parcel.writeByte(this.mB);

    }

    readFromParcel(parcel)
    {
        if (parcel == null)
            return;

        parcel.readString();

        this.mB = parcel.readByte();

    }
}

export class CList extends CBase
{
    constructor()
    {
        super();
        this.a = 0;
        this.b = 0;
        this.c = 0;
        this.e = 0;
        this.bee = true;
        this.f = null;
        this.g = null;
        this.m = null;
        this.lstring = [];
        this.list64 = [];
        this.listDouble = [];
    }

    writeToParcel(parcel)
    {
        if (parcel == null)
            return;

        parcel.writeString("NA.CList");

        super.writeToParcel(parcel);

        parcel.writeInt8(this.a);

        parcel.writeInt16(this.b);

        parcel.writeInt64(this.c);

        parcel.writeDouble(this.e);

        parcel.writeBoolean(this.bee);

        parcel.writeNullableInt8(this.f);

        parcel.writeNullableInt64(this.g);

        parcel.writeString(this.m);

        if (this.lstring) {
            parcel.writeInt32(this.lstring.length);
            for (var i = 0; i < this.lstring.length; i++) {
                parcel.writeString(this.lstring[i]);
            }
        }
        else {
            parcel.writeInt32(0);
        }

        if (this.list64) {
            parcel.writeInt32(this.list64.length);
            for (var i = 0; i < this.list64.length; i++) {
                parcel.writeNullableInt64(this.list64[i]);
            }
        }
        else {
            parcel.writeInt32(0);
        }

        if (this.listDouble) {
            parcel.writeInt32(this.listDouble.length);
            for (var i = 0; i < this.listDouble.length; i++) {
                parcel.writeNullableDouble(this.listDouble[i]);
            }
        }
        else {
            parcel.writeInt32(0);
        }

    }

    readFromParcel(parcel)
    {
        if (parcel == null)
            return;

        parcel.readString();

        super.readFromParcel(parcel);

        this.a = parcel.readInt8();

        this.b = parcel.readInt16();

        this.c = parcel.readInt64();

        this.e = parcel.readDouble();

        this.bee = parcel.readBoolean();

        this.f = parcel.readNullableInt8();

        this.g = parcel.readNullableInt64();

        this.m = parcel.readString();

        let list_lstring = [];
        let length_lstring = parcel.readInt32();
        for (let i = 0; i < length_lstring; i++) {
            let obj = parcel.readString();
            list_lstring.push(obj);
        }
        this.lstring = list_lstring;

        let list_list64 = [];
        let length_list64 = parcel.readInt32();
        for (let i = 0; i < length_list64; i++) {
            let obj = parcel.readNullableInt64();
            list_list64.push(obj);
        }
        this.list64 = list_list64;

        let list_listDouble = [];
        let length_listDouble = parcel.readInt32();
        for (let i = 0; i < length_listDouble; i++) {
            let obj = parcel.readNullableDouble();
            list_listDouble.push(obj);
        }
        this.listDouble = list_listDouble;

    }
}
</code></pre>
6. **生成java文件命令：./GenerateIDL.exe  -java test.idl**
<pre><code>
package NA;

import java.util.LinkedList;
import java.util.List;
import AXP.Parcel;
import AXP.AResult;

public class CList extends CBase
{
    public byte a;
    public short b;
    public long c;
    public double e;
    public boolean bee;
    public Byte f;
    public Long g;
    public String m;
    public List<String> lstring;
    public List<Long> list64;
    public List<Double> listDouble;

    public int WriteToParcel(Parcel parcel)
    {
        if (parcel == null)
            return AResult.AE_FAIL;

        if (AResult.AFAILED(parcel.WriteString(this.getClass().getName())))
            return AResult.AE_FAIL;

        if (AResult.AFAILED(super.WriteToParcel(parcel)))
            return AResult.AE_FAIL;

        if (AResult.AFAILED(parcel.WriteInt8(a)))
            return AResult.AE_FAIL;

        if (AResult.AFAILED(parcel.WriteInt16(b)))
            return AResult.AE_FAIL;

        if (AResult.AFAILED(parcel.WriteInt64(c)))
            return AResult.AE_FAIL;

        if (AResult.AFAILED(parcel.WriteDouble(e)))
            return AResult.AE_FAIL;

        if (AResult.AFAILED(parcel.WriteBoolean(bee)))
            return AResult.AE_FAIL;

        if (AResult.AFAILED(parcel.WriteNullableInt8(f)))
            return AResult.AE_FAIL;

        if (AResult.AFAILED(parcel.WriteNullableInt64(g)))
            return AResult.AE_FAIL;

        if (AResult.AFAILED(parcel.WriteString(m)))
            return AResult.AE_FAIL;

        {
            int length = 0;
            if (lstring == null)
                length = 0;
            else
                length = lstring.size();

            String typeStr = "L";
            byte[] type = typeStr.getBytes();
            if (AResult.AFAILED(parcel.Write(type)))
                return AResult.AE_FAIL;

            byte[] lengthArray = new byte[4];
            lengthArray[0] = (byte) (length >>> 0);
            lengthArray[1] = (byte) (length >>> 8);
            lengthArray[2] = (byte) (length >>> 16);
            lengthArray[3] = (byte) (length >>> 24);
            if (AResult.AFAILED(parcel.Write(lengthArray)))
                return AResult.AE_FAIL;

            if (lstring != null) {
                for(String obj : lstring) {
                    if (obj == null)
                        return AResult.AE_FAIL;

                    if (AResult.AFAILED(parcel.WriteString(obj)))
                        return AResult.AE_FAIL;
                }
            }
        }

        {
            int length = 0;
            if (list64 == null)
                length = 0;
            else
                length = list64.size();

            String typeStr = "L";
            byte[] type = typeStr.getBytes();
            if (AResult.AFAILED(parcel.Write(type)))
                return AResult.AE_FAIL;

            byte[] lengthArray = new byte[4];
            lengthArray[0] = (byte) (length >>> 0);
            lengthArray[1] = (byte) (length >>> 8);
            lengthArray[2] = (byte) (length >>> 16);
            lengthArray[3] = (byte) (length >>> 24);
            if (AResult.AFAILED(parcel.Write(lengthArray)))
                return AResult.AE_FAIL;

            if (list64 != null) {
                for(Long obj : list64) {
                    if (obj == null)
                        return AResult.AE_FAIL;

                    if (AResult.AFAILED(parcel.WriteNullableInt64(obj)))
                        return AResult.AE_FAIL;
                }
            }
        }

        {
            int length = 0;
            if (listDouble == null)
                length = 0;
            else
                length = listDouble.size();

            String typeStr = "L";
            byte[] type = typeStr.getBytes();
            if (AResult.AFAILED(parcel.Write(type)))
                return AResult.AE_FAIL;

            byte[] lengthArray = new byte[4];
            lengthArray[0] = (byte) (length >>> 0);
            lengthArray[1] = (byte) (length >>> 8);
            lengthArray[2] = (byte) (length >>> 16);
            lengthArray[3] = (byte) (length >>> 24);
            if (AResult.AFAILED(parcel.Write(lengthArray)))
                return AResult.AE_FAIL;

            if (listDouble != null) {
                for(Double obj : listDouble) {
                    if (obj == null)
                        return AResult.AE_FAIL;

                    if (AResult.AFAILED(parcel.WriteNullableDouble(obj)))
                        return AResult.AE_FAIL;
                }
            }
        }

        return AResult.AS_OK;
    }

    public int ReadFromParcel(Parcel parcel)
    {
        if (parcel == null)
            return AResult.AE_FAIL;

        try {
            parcel.ReadString();
            if (AResult.AFAILED(super.ReadFromParcel(parcel)))
                return AResult.AE_FAIL;

            a = parcel.ReadInt8();
            b = parcel.ReadInt16();
            c = parcel.ReadInt64();
            e = parcel.ReadDouble();
            bee = parcel.ReadBoolean();
            f = parcel.ReadNullableInt8();
            g = parcel.ReadNullableInt64();
            m = parcel.ReadString();
            lstring = new LinkedList<String>();
            try {
                byte[] type = parcel.Read(1);
                String typeStr = new String(type);
                if (!typeStr.equals("L"))
                    return AResult.AE_FAIL;

                byte [] lengthArray = parcel.Read(4);
                int length = (((lengthArray[0] << 0) & 0x000000ff) | ((lengthArray[1] << 8) & 0x0000ff00) | ((lengthArray[2] << 16) & 0x00ff0000) | ((lengthArray[3] << 24) & 0xff000000));
                for (int i = 0; i < length; i++) {
                    String obj = parcel.ReadString();
                    lstring.add(obj);
                }
            }
            catch (Exception e) {
                e.printStackTrace();
                return AResult.AE_FAIL;
            }

            list64 = new LinkedList<Long>();
            try {
                byte[] type = parcel.Read(1);
                String typeStr = new String(type);
                if (!typeStr.equals("L"))
                    return AResult.AE_FAIL;

                byte [] lengthArray = parcel.Read(4);
                int length = (((lengthArray[0] << 0) & 0x000000ff) | ((lengthArray[1] << 8) & 0x0000ff00) | ((lengthArray[2] << 16) & 0x00ff0000) | ((lengthArray[3] << 24) & 0xff000000));
                for (int i = 0; i < length; i++) {
                    Long obj = parcel.ReadNullableInt64();
                    list64.add(obj);
                }
            }
            catch (Exception e) {
                e.printStackTrace();
                return AResult.AE_FAIL;
            }

            listDouble = new LinkedList<Double>();
            try {
                byte[] type = parcel.Read(1);
                String typeStr = new String(type);
                if (!typeStr.equals("L"))
                    return AResult.AE_FAIL;

                byte [] lengthArray = parcel.Read(4);
                int length = (((lengthArray[0] << 0) & 0x000000ff) | ((lengthArray[1] << 8) & 0x0000ff00) | ((lengthArray[2] << 16) & 0x00ff0000) | ((lengthArray[3] << 24) & 0xff000000));
                for (int i = 0; i < length; i++) {
                    Double obj = parcel.ReadNullableDouble();
                    listDouble.add(obj);
                }
            }
            catch (Exception e) {
                e.printStackTrace();
                return AResult.AE_FAIL;
            }

        }
        catch (Exception e) {
            return AResult.AE_FAIL;
        }

        return AResult.AS_OK;
    }

    public Boolean Copy(CList info)
    {
        if (info == null)
            return false;

        super.Copy(info);

        a = info.a;
        b = info.b;
        c = info.c;
        e = info.e;
        bee = info.bee;
        f = info.f;
        g = info.g;
        m = info.m;

        if (info.lstring != null) {
            if (lstring != null)
                lstring.clear();
            else
                lstring = new LinkedList<String>();

            for (int i = 0; i < info.lstring.size(); ++i)
                lstring.add(info.lstring.get(i));
        }

        if (info.list64 != null) {
            if (list64 != null)
                list64.clear();
            else
                list64 = new LinkedList<Long>();

            for (int i = 0; i < info.list64.size(); ++i)
                list64.add(info.list64.get(i));
        }

        if (info.listDouble != null) {
            if (listDouble != null)
                listDouble.clear();
            else
                listDouble = new LinkedList<Double>();

            for (int i = 0; i < info.listDouble.size(); ++i)
                listDouble.add(info.listDouble.get(i));
        }

        return true;
    }

    public void SetNull()
    {
        super.SetNull();

        a = 0;
        b = 0;
        c = 0;
        e = 0;
        bee = false;
        f = null;
        g = null;
        m = null;
        if (lstring != null)
            lstring.clear();

        if (list64 != null)
            list64.clear();
        if (listDouble != null)
            listDouble.clear();
    }

    public String ToString()
    {
        String json = "{";
        json = String.format("%s\"a\":\"%s\",", json, a);
        if (json == null)
            return null;

        json = String.format("%s\"b\":\"%s\",", json, b);
        if (json == null)
            return null;

        json = String.format("%s\"c\":\"%s\",", json, c);
        if (json == null)
            return null;

        json = String.format("%s\"e\":\"%s\",", json, e);
        if (json == null)
            return null;

        json = String.format("%s\"bee\":\"%s\",", json, bee);
        if (json == null)
            return null;

        if (f == null) {
            json = String.format("%s\"f\":\"\",", json);
            if (json == null)
                return null;
        }
        else {
            json = String.format("%s\"f\":\"%s\",", json, f);
            if (json == null)
                return null;
        }

        if (g == null) {
            json = String.format("%s\"g\":\"\",", json);
            if (json == null)
                return null;
        }
        else {
            json = String.format("%s\"g\":\"%s\",", json, g);
            if (json == null)
                return null;
        }

        if (m == null) {
            json = String.format("%s\"m\":\"\",", json);
            if (json == null)
                return null;
        }
        else {
            json = String.format("%s\"m\":\"%s\",", json, m);
            if (json == null)
                return null;
        }

        if (lstring == null) {
            json = String.format("%s\"lstring\":[],", json);
            if (json == null)
                return null;
        }
        else {
            String jsonTmp = String.format("\"lstring\":[");
            if (jsonTmp == null)
                return null;

            for(int i = 0; i < lstring.size(); ++i) {
                String obj = lstring.get(i);
                String comma = null;
                if (i < lstring.size() - 1)
                    comma = ",";
                else
                    comma = "";

                jsonTmp = String.format("%s\"%s\"%s", jsonTmp, obj, comma);
                if (jsonTmp == null)
                    return null;
            }

            jsonTmp = String.format("%s],", jsonTmp);
            if (jsonTmp == null)
                return null;

            json = String.format("%s%s", json, jsonTmp);
            if (json == null)
                return null;
        }

        if (list64 == null) {
            json = String.format("%s\"list64\":[],", json);
            if (json == null)
                return null;
        }
        else {
            String jsonTmp = String.format("\"list64\":[");
            if (jsonTmp == null)
                return null;

            for(int i = 0; i < list64.size(); ++i) {
                Long obj = list64.get(i);
                String comma = null;
                if (i < list64.size() - 1)
                    comma = ",";
                else
                    comma = "";

                jsonTmp = String.format("%s\"%s\"%s", jsonTmp, obj, comma);
                if (jsonTmp == null)
                    return null;
            }

            jsonTmp = String.format("%s],", jsonTmp);
            if (jsonTmp == null)
                return null;

            json = String.format("%s%s", json, jsonTmp);
            if (json == null)
                return null;
        }

        if (listDouble == null) {
            json = String.format("%s\"listDouble\":[]", json);
            if (json == null)
                return null;
        }
        else {
            String jsonTmp = String.format("\"listDouble\":[");
            if (jsonTmp == null)
                return null;

            for(int i = 0; i < listDouble.size(); ++i) {
                Double obj = listDouble.get(i);
                String comma = null;
                if (i < listDouble.size() - 1)
                    comma = ",";
                else
                    comma = "";

                jsonTmp = String.format("%s\"%s\"%s", jsonTmp, obj, comma);
                if (jsonTmp == null)
                    return null;
            }

            jsonTmp = String.format("%s]", jsonTmp);
            if (jsonTmp == null)
                return null;

            json = String.format("%s%s", json, jsonTmp);
            if (json == null)
                return null;
        }

        return String.format("%s}", json);
    }

    public String GetTypeName()
    {
        return "NA.CList";
    }
}
</code></pre>