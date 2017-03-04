
"use strict";

import * as NA from "./NA/test";

sServiceTable = {};
sMaxIndex = 0;

export class ClassLoader
{
    static createObject(className)
    {
        return eval("new " + className + "();");
    }

    static getService(remoteRefCode)
    {
        return sServiceTable[remoteRefCode];
    }

    static registerService(serviceRef)
    {
        sServiceTable[++sMaxIndex] = serviceRef;
        return sMaxIndex;
    }

    static removeService(remoteRefCode)
    {
        delete sServiceTable[remoteRefCode];
    }
}
