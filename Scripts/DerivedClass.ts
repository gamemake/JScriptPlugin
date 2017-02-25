
import {blueprint, updateBPInfo, getBPInfo, getClassName, getParentClassName} from './Blueprint';
import BaseClass from './BaseClass';

@blueprint
export default class DerivedClass extends BaseClass {
    @blueprint
    static staticField: string;
    private staticField1: string;

    @blueprint
    static staticMethod() {
    }

    private static staticMethod1() {
    }

    @blueprint
    field: string;

    private field1: string;

    @blueprint
    method() {

    }

    private method1() {

    }
}

console.log("Class Name        :", getClassName(DerivedClass));
console.log("Parent Class Name :", getParentClassName(DerivedClass));
console.log("Class Name        :", getClassName(BaseClass));
console.log("Parent Class Name :", getParentClassName(BaseClass));

declare var require: any;

updateBPInfo(require('fs').readFileSync('a.d.ts', 'utf8').split('\n'));
console.log("bpClassInfo       :", JSON.stringify(getBPInfo(DerivedClass), null, 4));
