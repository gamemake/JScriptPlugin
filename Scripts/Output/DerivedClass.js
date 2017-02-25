"use strict";
var __extends = (this && this.__extends) || function (d, b) {
    for (var p in b) if (b.hasOwnProperty(p)) d[p] = b[p];
    function __() { this.constructor = d; }
    d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
};
var __decorate = (this && this.__decorate) || function (decorators, target, key, desc) {
    var c = arguments.length, r = c < 3 ? target : desc === null ? desc = Object.getOwnPropertyDescriptor(target, key) : desc, d;
    if (typeof Reflect === "object" && typeof Reflect.decorate === "function") r = Reflect.decorate(decorators, target, key, desc);
    else for (var i = decorators.length - 1; i >= 0; i--) if (d = decorators[i]) r = (c < 3 ? d(r) : c > 3 ? d(target, key, r) : d(target, key)) || r;
    return c > 3 && r && Object.defineProperty(target, key, r), r;
};
var Blueprint_1 = require("./Blueprint");
var BaseClass_1 = require("./BaseClass");
var DerivedClass = (function (_super) {
    __extends(DerivedClass, _super);
    function DerivedClass() {
        return _super !== null && _super.apply(this, arguments) || this;
    }
    DerivedClass.staticMethod = function () {
    };
    DerivedClass.staticMethod1 = function () {
    };
    DerivedClass.prototype.method = function () {
    };
    DerivedClass.prototype.method1 = function () {
    };
    return DerivedClass;
}(BaseClass_1.default));
__decorate([
    Blueprint_1.blueprint
], DerivedClass.prototype, "field", void 0);
__decorate([
    Blueprint_1.blueprint
], DerivedClass.prototype, "method", null);
__decorate([
    Blueprint_1.blueprint
], DerivedClass, "staticField", void 0);
__decorate([
    Blueprint_1.blueprint
], DerivedClass, "staticMethod", null);
DerivedClass = __decorate([
    Blueprint_1.blueprint
], DerivedClass);
Object.defineProperty(exports, "__esModule", { value: true });
exports.default = DerivedClass;
console.log("Class Name        :", Blueprint_1.getClassName(DerivedClass));
console.log("Parent Class Name :", Blueprint_1.getParentClassName(DerivedClass));
console.log("Class Name        :", Blueprint_1.getClassName(BaseClass_1.default));
console.log("Parent Class Name :", Blueprint_1.getParentClassName(BaseClass_1.default));
Blueprint_1.updateBPInfo(require('fs').readFileSync('a.d.ts', 'utf8').split('\n'));
console.log("bpClassInfo       :", JSON.stringify(Blueprint_1.getBPInfo(DerivedClass), null, 4));
//# sourceMappingURL=data:application/json;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoiRGVyaXZlZENsYXNzLmpzIiwic291cmNlUm9vdCI6IiIsInNvdXJjZXMiOlsiLi4vRGVyaXZlZENsYXNzLnRzIl0sIm5hbWVzIjpbXSwibWFwcGluZ3MiOiI7Ozs7Ozs7Ozs7OztBQUNBLHlDQUFpRztBQUNqRyx5Q0FBb0M7QUFHcEMsSUFBcUIsWUFBWTtJQUFTLGdDQUFTO0lBQW5EOztJQXlCQSxDQUFDO0lBbkJVLHlCQUFZLEdBQW5CO0lBQ0EsQ0FBQztJQUVjLDBCQUFhLEdBQTVCO0lBQ0EsQ0FBQztJQVFELDZCQUFNLEdBQU47SUFFQSxDQUFDO0lBRU8sOEJBQU8sR0FBZjtJQUVBLENBQUM7SUFDTCxtQkFBQztBQUFELENBQUMsQUF6QkQsQ0FBMEMsbUJBQVMsR0F5QmxEO0FBWkc7SUFEQyxxQkFBUzsyQ0FDSTtBQUtkO0lBREMscUJBQVM7MENBR1Q7QUFsQkQ7SUFEQyxxQkFBUzt1Q0FDaUI7QUFJM0I7SUFEQyxxQkFBUztzQ0FFVDtBQVBnQixZQUFZO0lBRGhDLHFCQUFTO0dBQ1csWUFBWSxDQXlCaEM7O2tCQXpCb0IsWUFBWTtBQTJCakMsT0FBTyxDQUFDLEdBQUcsQ0FBQyxxQkFBcUIsRUFBRSx3QkFBWSxDQUFDLFlBQVksQ0FBQyxDQUFDLENBQUM7QUFDL0QsT0FBTyxDQUFDLEdBQUcsQ0FBQyxxQkFBcUIsRUFBRSw4QkFBa0IsQ0FBQyxZQUFZLENBQUMsQ0FBQyxDQUFDO0FBQ3JFLE9BQU8sQ0FBQyxHQUFHLENBQUMscUJBQXFCLEVBQUUsd0JBQVksQ0FBQyxtQkFBUyxDQUFDLENBQUMsQ0FBQztBQUM1RCxPQUFPLENBQUMsR0FBRyxDQUFDLHFCQUFxQixFQUFFLDhCQUFrQixDQUFDLG1CQUFTLENBQUMsQ0FBQyxDQUFDO0FBSWxFLHdCQUFZLENBQUMsT0FBTyxDQUFDLElBQUksQ0FBQyxDQUFDLFlBQVksQ0FBQyxRQUFRLEVBQUUsTUFBTSxDQUFDLENBQUMsS0FBSyxDQUFDLElBQUksQ0FBQyxDQUFDLENBQUM7QUFDdkUsT0FBTyxDQUFDLEdBQUcsQ0FBQyxxQkFBcUIsRUFBRSxJQUFJLENBQUMsU0FBUyxDQUFDLHFCQUFTLENBQUMsWUFBWSxDQUFDLEVBQUUsSUFBSSxFQUFFLENBQUMsQ0FBQyxDQUFDLENBQUMifQ==