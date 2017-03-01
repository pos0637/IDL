
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
