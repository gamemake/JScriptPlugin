
export class BPFieldInfo {
    Static: boolean;
    Name: string;
    Type: string;
    IsArray: boolean;

    constructor(Static: boolean, Name: string, Type?: string, IsArray?: boolean) {
        this.Static = Static;
        this.Type = Type;
        this.Name = Name;
        this.Type = Type?Type:"";
        this.IsArray = IsArray?true:false;
        if(typeof Type!=='undefined' && typeof IsArray==='undefined') {
            this.setType(Type);
        }
    }

    setType(Type: string): void {
        if(Type.substr(Type.length-2)==='[]') {
            this.Type = Type.substr(0, Type.length-2);
            this.IsArray = true;
        }
    }
};

export class BPMethodInfo {
    Static: boolean;
    Name: string;
    Parameters: BPFieldInfo[];
    ReturnValue: BPFieldInfo;

    constructor(Static: boolean, Name: string) {
        this.Static = Static;
        this.Name = typeof Name === 'string'?Name:"";
        this.Parameters = [];
        this.ReturnValue = null;
    }
}

export class BPClassInfo {
    Class: string;
    ParentClass: string;
    Members: BPFieldInfo[];
    Methods: BPMethodInfo[];

    constructor() {
        this.Class = "";
        this.ParentClass = "";
        this.Members = [];
        this.Methods = [];
    }

    addMember(FieldInfo: BPFieldInfo): void {
        this.Members.push(FieldInfo);
    }

    addMethod(MethodInfo: BPMethodInfo) {
        this.Methods.push(MethodInfo);
    }

    getMember(Static: boolean, Name: string) : BPFieldInfo {
        for(var i=0; i<this.Members.length; i++) {
            if(this.Members[i].Name==name && this.Members[i].Static==Static) {
                return this.Members[i];
            }
        }
        return null;
    }

    getMethod(Static: boolean, Name: string) : BPMethodInfo {
        for(var i=0; i<this.Methods.length; i++) {
            if(this.Methods[i].Name==name && this.Methods[i].Static==Static) {
                return this.Methods[i];
            }
        }
        return null;
    }
};

export function getClassName(obj: Function | Object | any) : string {
    if(typeof obj === 'function') {
        return obj.prototype.constructor.name;
    } else {
        return Object.getPrototypeOf(obj).constructor.name;
    }
}

export function getParentClassName(obj: Function | Object) : string {
    if(typeof obj === 'function') {
        return Object.getPrototypeOf(obj.prototype).constructor.name;
    } else {
        return Object.getPrototypeOf(Object.getPrototypeOf(obj)).constructor.name;
    }
}

var BPClassMap : {[key: string]: BPClassInfo} = {}

export function blueprint(target: Function | Object | any, name?: string) : void {
    if(typeof target === 'function') {
        var bp: BPClassInfo = target.prototype.bpInfo?target.prototype.bpInfo:(target.prototype.bpInfo=new BPClassInfo);
        if(typeof name === 'undefined') {
            bp.Class = getClassName(target);
            bp.ParentClass = getParentClassName(target);
            BPClassMap[bp.Class] = bp;
        } else {
            if(typeof target[name] === 'function') {
                bp.addMethod(new BPMethodInfo(true, name));
            } else {
                bp.addMember(new BPFieldInfo(true, name));
            }
        }
    } else {
        var bp: BPClassInfo = target.bpInfo?target.bpInfo:(target.bpInfo=new BPClassInfo);
        if(typeof target[name] === 'function') {
            bp.addMethod(new BPMethodInfo(false, name));
        } else {
            bp.addMember(new BPFieldInfo(false, name));
        }
    }
}

export function getBPInfo(obj: Function | Object | string) : BPClassInfo {
    if(typeof obj === 'function') {
        var bp = obj.prototype.bpInfo;
        return Object.getPrototypeOf(bp).constructor.name === 'BPClassInfo'?bp:null;
    } else if(typeof obj === 'object') {
        var bp = Object.getPrototypeOf(obj).bpInfo;
        return Object.getPrototypeOf(bp).constructor.name === 'BPClassInfo'?bp:null;
    } else if(typeof obj === 'string') {
        if(BPClassMap.hasOwnProperty(obj)) {
            return BPClassMap[obj];
        }
    }
    return null;
}

export function updateBPInfo(lines: string[]) : void {
    var bpClass: BPClassInfo = null;

    for(var lineno=0; lineno<lines.length; lineno++) {
        if(!bpClass) {
            var words = lines[lineno].trim().split(' ');
            var start = 0;
            if(words[start]==='export') start += 1;
            if(words[start]==='default') start += 1;
            if(words[start]==='class') {
                var className: string;
                var parentName: string;
                if(words[start + 2]==='extends') {
                    className = words[start + 1];
                    parentName = words[start + 3];
                } else {
                    className = words[start + 1];
                    parentName = "";
                }
                bpClass = getBPInfo(className);
            }
            continue;
        }

        var line = lines[lineno].trim();
        if(line=='}') {
            bpClass = null;
            continue;
        }
        line = line.substring(0, line.length-1);

        var isStatic = false;
        if(line.indexOf('static ')==0) {
            line = line.substr(7);
            isStatic = true;
        }

        var pos = line.lastIndexOf(':');
        if(pos<0) continue;
        var stype = line.substr(pos + 1).trim();
        line = line.substr(0, pos).trim();

        pos = line.indexOf('(');
        if(pos<=0) {
            var member = bpClass.getMember(isStatic, line);
            if(member) {
                member.setType(stype);
            }
        } else {
            var method = bpClass.getMethod(isStatic, line.substr(0, pos));
            if(method) {
                if(stype!=='void') {
                    method.ReturnValue = new BPFieldInfo(false, "", stype);
                }

                var sparam = line.substr(pos+1, line.length - pos - 1 - 1).split(',');
                for(var i=0; i<sparam.length; i++) {
                    if(sparam[i].length>0) {
                        pos = sparam[i].indexOf(':');
                        method.Parameters.push(new BPFieldInfo(false, sparam[i].substr(0, pos).trim(), sparam[i].substr(pos + 1).trim()));
                    }
                }
            }
        }
    }
}

export function GetAllBPClassInfo() : BPClassInfo[] {
    var retval: BPClassInfo[] = [];
    for (var name in BPClassMap) {
        if(!BPClassMap.hasOwnProperty(name)) continue;
        retval.push(BPClassMap[name]);
    }
    return retval;
}
