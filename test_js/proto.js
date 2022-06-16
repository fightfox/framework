function A(x) {
  this.x = x;
  this.say = function() {
    console.log("HAHAHA");
  }
}

A.prototype.say = function () {
  console.log(this.x);
}

var aa = new A();

function B(y) {
  A.call(this, y); // actuall attributes inheritance happened
}

B.prototype = Object.create(A.prototype); // function inheritance
// B.prototype = new A(); // function inheritance

// {x, say} -> {_proto_: A.prototype} -> A.prototype = {say: {log this.x}} -> {} -> null
// {x, say} -> {x, sayHAHAHA, _proto_: A.prototype} -> A.prototype = {say: {log this.x}} -> {} -> null

var c = Object.create(B.prototype);
c.say();

// {__proto__: B.prototype={A.prototype}/new A()} -> 

var t = new B(33);
// {}
t.say();

var t2 = new B(35);
t2.say = function() {
  console.log("This is t2");
}
t2.say();
t.say();

var obj = {};
console.log((new A()).__proto__===A.prototype );

var ttt = new A(3333);
ttt.say();

// Object.create(A.prototype)
// // {__proto__: A.prototype}
// new A()
// // {__proto__:A.prototype, x:...}
