var canvas = document.getElementById("canvas");
var stage, arena;
var units = [];
var data = [];
var ws;
var ws_address = "ws://45.79.152.5:3000"; 
var local_address = "ws://localhost:3000";
var hint;
var draft = null;
var test2;
var operation = 999;
var my_id = -1;

function getMouse() {
  return {x: stage.mouseX-canvas.width/2, y: stage.mouseY-canvas.height/2};
}

function getArenaXY(x, y) {
  if (my_id!=-1) {
    var unit_id = data[my_id].my_unit;
    return {x: x-canvas.width/2+units[unit_id].x, y: y-canvas.height/2+units[unit_id].y};
  }
  else
    return {x: x, y: y};
}

function Unit(type, id, x = 0, y = 0, r = 0, vx = 0, vy = 0) {
  this.type = type;
  this.id = id;
  this.x = x;
  this.y = y;
  this.r = r;
  this.vx = vx;
  this.vy = vy;
  this.shape = null;
  this.label = null;
}

Unit.prototype.getShape = function() {
  if (this.shape==null) {
    this.shape = new createjs.Shape();
    arena.addChild(this.shape);
  } 
  return this.shape;
}
Unit.prototype.moveTo = function(_x, _y) {
  this.x = _x;
  this.y = _y;
}
Unit.prototype.setV = function(_x, _y) {
  this.vx = _x-this.x;
  this.vy = _y-this.y;
}
Unit.prototype.setR = function(_r) {
  this.r = _r;
}
Unit.prototype.drawLabel = function(txt) {
  if (this.label==null) {
    this.label = new createjs.Text(txt);
    arena.addChild(this.label);
  }
  else
    this.label.text = txt;
  this.label.x = this.x;
  this.label.y = this.y;
}
Unit.prototype.receiveXY = function(reader) {
  this.x = reader.readFloat();
  this.y = reader.readFloat();
}
Unit.prototype.receiveV = function(reader) {
  this.vx = reader.readFloat();
  this.vy = reader.readFloat();
}
Unit.prototype.moveUnit = function(reader) {
  this.x = reader.readFloat();
  this.y = reader.readFloat();
}
Unit.prototype.clearImage = function() {
  if (this.shape!=null)
    arena.removeChild(this.shape);
  if (this.label!=null)
    arena.removeChild(this.label);
  this.shape = null;
  this.label = null;
}

function Circle(id, x = 0, y = 0, r = 0, vx = 0, vy = 0) {
  Unit.call(this, "CIRCLE", id, x, y, r, vx, vy);
}
Circle.prototype = Object.create(Unit.prototype);
Circle.prototype.draw = function() {
  var shape = this.getShape();
  shape.graphics.clear().beginStroke("#000").drawCircle(this.x, this.y, this.r);
}
Circle.prototype.resize = function(_x, _y) {
  var xx = _x-this.x, yy = _y-this.y;
  this.r = Math.sqrt(xx*xx+yy*yy);
}
Circle.prototype.send = function() {
  var writer = new BufferWriter();
  writer.writeChar(1);
  writer.writeFloat(this.x);
  writer.writeFloat(this.y);
  writer.writeFloat(this.r);
  writer.writeFloat(Math.atan2(this.vx, this.vy));
  ws.send(writer.compile());
}
Circle.prototype.showUnit = function(reader) {
  this.x = reader.readFloat();
  this.y = reader.readFloat();
  this.r = reader.readFloat();
}

function Segment(id, x0 = 0, y0 = 0, x1 = 0, y1 = 0, r = 0, vx = 0, vy = 0) {
  Unit.call(this, "SEGMENT", id, (x0+x1)/2, (y0+y1)/2, r, vx, vy);
  this.dx = x0-this.x;
  this.dy = y0-this.y;
}
Segment.prototype = Object.create(Unit.prototype);
Segment.prototype.draw = function() {
  var shape = this.getShape();
  shape.graphics.clear().setStrokeStyle(this.r*2, "round", "round").beginStroke("#000")
    .moveTo(this.x+this.dx, this.y+this.dy)
    .lineTo(this.x-this.dx, this.y-this.dy); 
  arena.addChild(shape);
}
Segment.prototype.resize = function(_x, _y) {
  var x1 = this.x-this.dx, y1 = this.y-this.dy;
  this.moveTo((_x+x1)/2, (_y+y1)/2);
  this.dx = _x-this.x;
  this.dy = _y-this.y;
}
Segment.prototype.send = function() {
  var writer = new BufferWriter();
  writer.writeChar(2);
  writer.writeFloat(this.x+this.dx);
  writer.writeFloat(this.y+this.dy);
  writer.writeFloat(this.x-this.dx);
  writer.writeFloat(this.y-this.dy);
  writer.writeFloat(this.r);
  ws.send(writer.compile());
}
Segment.prototype.showUnit = function(reader) {
  var x0 = reader.readFloat(), 
    y0 = reader.readFloat(),
    x1 = reader.readFloat(),
    y1 = reader.readFloat();
  this.r = reader.readFloat();
  this.x = (x0+x1)/2;
  this.y = (y0+y1)/2;
  this.dx = x0-this.x;
  this.dy = y0-this.y;
}

function handle_action(case_id, reader) {
  var id, type_id, shape_id;
  switch (case_id) {
    case 1:   // showUnit
      id = reader.readInt();
      type_id = reader.readChar();
      if (!data[id]) data[id] = {}
      data[id].type = type_id;
      shape_id = reader.readChar();
      if (!units[id]) {
        if (shape_id==0)
          units[id] = new Circle(id);
        else if (shape_id==1)
          units[id] = new Segment(id); 
      }
      units[id].showUnit(reader);
      break;
    case 2:   // moveUnit
      id = reader.readInt();
      units[id].moveUnit(reader);
      break;
    case 3:   // createUnit
      id = reader.readInt();
      break;
    case 4:   // removePlayer
      id = reader.readInt();
      break;
    case 5:   // createPlayer
      //TODO
      my_id = reader.readInt();
      console.log("Create player ", my_id);
      operation = 1;
      break;
    case 6:   // removeUnit
      id = reader.readInt();
      break;
    case 7:   // changeUnit (Figure)
      id = reader.readInt();
      units[id].clearImage();
      shape_id = reader.readChar();
      if (shape_id==0)
        units[id] = new Circle(id);
      else if (shape_id==1)
        units[id] = new Segment(id); 
      units[id].showUnit(reader);
      break;
    case 8:   // changeUnit (R)
      id = reader.readInt();
      units[id].setR(reader.readFloat());
      break;
    case 9:   // changeUnit (XY)
      id = reader.readInt();
      units[id].receiveXY(reader);
      break;
    case 10:  // hideUnit
      id = reader.readInt();
      units[id].clearImage();
      delete units[id];
      break;
    case 11:  // information completion
      id = reader.readInt();
      var old_priv = reader.readChar(), new_priv = reader.readChar(), data_type = readChar();
      //console.log(id, old_priv, new_priv, "information");
      if (!data[id])
        data[id] = {};
      if (id % 2==1) {
        for (var priv = old_priv+1; priv <=new_priv; priv++) {
          if (priv==1) {
            data[id].nickname = reader.readString();
            //console.log(data[id].nickname, "nickname");
          }
          else if (priv==2) {
            data[id].my_unit = reader.readInt();
            //console.log(data[id].my_unit, "my unit");
          }
        } 
      }
      else {
        //console.log(id, old_priv, new_priv, data[id].type);
        if (data_type==1)
          break;
        for (var priv = old_priv+1; priv <=new_priv; priv++) {
          if (priv==1) {
            data[id].owner = reader.readInt();
            //console.log(data[id].owner, "owner");
          }
        } 
      }
      break;
    case 101:   // update player's my_unit
      id = reader.readInt();
      data[id].my_unit = reader.readInt();
      break;
    case 102:   // update unit's owner
      id = reader.readInt();
      data[id].owner = reader.readInt();
      break;
  }
}

function setup_socket() {
  if (document.getElementById("nickname").value=="local")
    ws_address = local_address;
  ws = new WebSocket(ws_address);
  console.log("websocket");
  ws.binaryType = "arraybuffer";
  ws.onmessage = function (message) {
    var reader = new BufferReader(message);
    var case_id;
    while ((case_id = reader.readChar())!=99)
      handle_action(case_id, reader); 
  };
  ws.onopen = function () {
    var writer = new BufferWriter();
    writer.writeChar(0);
    var my_name = document.getElementById("nickname").value;
    writer.writeString(my_name);
    ws.send(writer.compile());
  };
  ws.onclose = function() {
    
  };
  ws.onerror = function(event) {
    gameConsole.log("socket error: ");
    gameConsole.log(event);
  };
}

function tick() {
  for (var i in units) {
    var unit = units[i];
    unit.draw();
    if (data[i] && data[i].type==0 && data[i].owner!=-1)
      unit.drawLabel(data[data[i].owner].nickname);  
  }
  if (draft!=null) {
    var p = getArenaXY(stage.mouseX, stage.mouseY);
    draft.resize(p.x, p.y);
    if (draft.type=="CIRCLE")
      draft.setV(p.x, p.y);
    draft.draw();
  }
  hint.text = "Operation = "+operation;
  if (my_id!=-1) {
    var unit_id = data[my_id].my_unit;
    arena.x = canvas.width/2-units[unit_id].x;
    arena.y = canvas.height/2-units[unit_id].y; 
  }
  stage.update();
  if (operation==3) {
    var writer = new BufferWriter();
    writer.writeChar(5);
    var my_mouse = getMouse();
    writer.writeFloat(my_mouse.x);
    writer.writeFloat(my_mouse.y);
    ws.send(writer.compile());
  }
}


function start() {
  document.getElementById("login").style.display = "none";
  setup_socket();
  stage.addEventListener("stagemousedown", function (evt) {
    var p = getArenaXY(stage.mouseX, stage.mouseY);
    if (operation==1)
      draft = new Circle(0, p.x, p.y, 0, 0, 0);
    else if (operation==2)
      draft = new Segment(0, p.x, p.y, p.x, p.y, 10, 0, 0);
  });

  stage.addEventListener("stagemouseup", function (evt) {
    if (draft!=null && (operation==1 || operation==2)) {
      draft.send();
      draft.clearImage();
      draft = null;
    }
  });
}

document.getElementById("play_online").onclick = start;

function init() {
  canvas.width = window.innerWidth;
  canvas.height = window.innerHeight;
  window.onresize = onResize;
  stage = new createjs.Stage(canvas);
  arena = new createjs.Container();
  stage.addChild(arena);
  stage.addChild(hint = new createjs.Text("Operation = "+operation, "bold 24px Arial"));
  hint.x = 10;
  hint.y = 10;

  createjs.Ticker.timingMode = createjs.Ticker.RAF;
  createjs.Ticker.addEventListener("tick", tick);
  document.onkeydown = keyPressed;

  canvas.style.display = "block";
}

function onResize() {
  canvas.width = window.innerWidth;
  canvas.height = window.innerHeight;
  stage.update();
}

function keyPressed(evt) {
  switch(event.keyCode) {
    case 48: // 0
      var writer = new BufferWriter();
      writer.writeChar(0);
      ws.send(writer.compile());
      break;
    case 49: // 1
      operation = 1;
      break;
    case 50: // 2
      operation = 2;
      break;
    case 51: // enable mouse
      operation = 3;
      break;
    case 52: // 4
      var writer = new BufferWriter();
      writer.writeChar(4);
      var p = getArenaXY(stage.mouseX, stage.mouseY);
      writer.writeFloat(p.x);
      writer.writeFloat(p.y);
      ws.send(writer.compile());
      break;
    case 32: // SPACE
      var writer = new BufferWriter();
      writer.writeChar(3);
      ws.send(writer.compile());
      break;
  }
}


//=============================================================================
function BufferReader(message) {
  this.data = new DataView(message.data);
  this.offset = 0;
}
// prototype of BufferReader
BufferReader.prototype.readInt = function () {
  var ans = this.data.getInt32(this.offset, true);  // true <-> little-endian
  this.offset += 4;
  return ans;
};
BufferReader.prototype.readFloat = function () {
  var ans = this.data.getFloat32(this.offset, true);
  this.offset += 4;
  return ans;
};
BufferReader.prototype.readInt64 = function () {
  return this.readInt() + this.readInt() * 4294967296 ;
};
BufferReader.prototype.readChar = function () {
  var ans = this.data.getUint8(this.offset, true);
  this.offset += 1;
  return ans;
};
BufferReader.prototype.readString = function () {
  var ans = "";
  var n = this.readChar()/2;
  for (var i = 0; i < n; ++i ) {
    ans += String.fromCharCode(this.data.getUint16(this.offset, true));
    this.offset += 2;
  }
  return ans;
};

//=============================================================================
function BufferWriter() {
  this.bytes = [];
}
BufferWriter.prototype.writeChar = function (ch) {
  this.bytes.push(ch);
};
BufferWriter.prototype.writeFloat = function (fl) {
  var data = new ArrayBuffer(4);
  var view = new DataView(data);
  view.setFloat32(0, fl, true);
  this.bytes.push(view.getUint8(0));
  this.bytes.push(view.getUint8(1));
  this.bytes.push(view.getUint8(2));
  this.bytes.push(view.getUint8(3));
};
BufferWriter.prototype.writeInt = function (i) {
  var data = new ArrayBuffer(4);
  var view = new DataView(data);
  view.setInt32(0, i, true);
  this.bytes.push(view.getUint8(0));
  this.bytes.push(view.getUint8(1));
  this.bytes.push(view.getUint8(2));
  this.bytes.push(view.getUint8(3));
};
BufferWriter.prototype.writeInt64 = function (t) {
  var base = 4294967296;
  this.writeInt(t % base);
  this.writeInt(~~(t / base));
};
BufferWriter.prototype.writeString = function (s) {
  this.bytes.push(s.length*2);
  var data = new ArrayBuffer(2);
  var view = new DataView(data);
  for (var i = 0; i < s.length; ++i ) {
    view.setUint16(0, s.charCodeAt(i), true);
    this.bytes.push(view.getUint8(0));
    this.bytes.push(view.getUint8(1));
  }
};
BufferWriter.prototype.compile = function () {
  var data = new ArrayBuffer(this.bytes.length);
  var view = new DataView(data);
  for (var i = 0; i < this.bytes.length; ++i )
    view.setUint8(i, this.bytes[i]);
  return data;
};
//=============================================================================