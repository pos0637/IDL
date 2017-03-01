
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
