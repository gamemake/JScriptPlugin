import BaseClass from './BaseClass';
export default class DerivedClass extends BaseClass {
    static staticField: string;
    private staticField1;
    static staticMethod(): void;
    private static staticMethod1();
    field: string;
    private field1;
    method(): void;
    private method1();
}
