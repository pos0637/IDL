
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
