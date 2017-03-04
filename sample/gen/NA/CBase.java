
package NA;

import AXP.IParcelable;
import AXP.Parcel;
import AXP.AResult;

public class CBase implements IParcelable
{
    public byte mB;

    public int WriteToParcel(Parcel parcel)
    {
        if (parcel == null)
            return AResult.AE_FAIL;

        if (AResult.AFAILED(parcel.WriteString(this.getClass().getName())))
            return AResult.AE_FAIL;

        if (AResult.AFAILED(parcel.WriteByte(mB)))
            return AResult.AE_FAIL;

        return AResult.AS_OK;
    }

    public int ReadFromParcel(Parcel parcel)
    {
        if (parcel == null)
            return AResult.AE_FAIL;

        try {
            parcel.ReadString();
            mB = parcel.ReadByte();
        }
        catch (Exception e) {
            return AResult.AE_FAIL;
        }

        return AResult.AS_OK;
    }

    public Boolean Copy(CBase info)
    {
        if (info == null)
            return false;

        mB = info.mB;

        return true;
    }

    public void SetNull()
    {
        mB = 0;
    }

    public String ToString()
    {
        return String.format("%s", mB);
    }

    public String GetTypeName()
    {
        return "NA.CBase";
    }
}
