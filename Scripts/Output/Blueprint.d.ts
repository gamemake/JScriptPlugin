export declare class BPFieldInfo {
    Static: boolean;
    Name: string;
    Type: string;
    IsArray: boolean;
    constructor(Static: boolean, Name: string, Type?: string, IsArray?: boolean);
    setType(Type: string): void;
}
export declare class BPMethodInfo {
    Static: boolean;
    Name: string;
    Parameters: BPFieldInfo[];
    ReturnValue: BPFieldInfo;
    constructor(Static: boolean, Name: string);
}
export declare class BPClassInfo {
    Class: string;
    ParentClass: string;
    Members: BPFieldInfo[];
    Methods: BPMethodInfo[];
    constructor();
    addMember(FieldInfo: BPFieldInfo): void;
    addMethod(MethodInfo: BPMethodInfo): void;
    getMember(Static: boolean, Name: string): BPFieldInfo;
    getMethod(Static: boolean, Name: string): BPMethodInfo;
}
export declare function getClassName(obj: Function | Object | any): string;
export declare function getParentClassName(obj: Function | Object): string;
export declare function blueprint(target: Function | Object | any, name?: string): void;
export declare function getBPInfo(obj: Function | Object | string): BPClassInfo;
export declare function updateBPInfo(lines: string[]): void;
export declare function GetAllBPClassInfo(): BPClassInfo[];
