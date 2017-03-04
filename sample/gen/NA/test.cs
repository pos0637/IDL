
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
