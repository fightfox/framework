var BOT_NUM = 200;
var time_out = 10;
var WebSocket = require('uws');
var bot_count = 0;
var bots = [];
function Bot() {
  this.units = [];
  this.data = [];
  this.ws;
  this.ws_address = "ws://45.79.128.170:3000";
  this.local_address = "ws://localhost:3000";
  this.hint = {};
  this.debug_hint = {};
  this.draft = null;
  this.test2;
  this.operation = 999;
  this.debug_info = ""; 
  this.current_tick_time = 0.0;
  this.my_id = -1;
  this.dir = 0;
  this.adir = 0;
  ws = new WebSocket(this.ws_address);
  ws.binaryType = "arraybuffer";
  var self = this;
  ws.onmessage = function (message) {
    var reader = new BufferReader(message);
    var case_id;
    while ((case_id = reader.readChar())!=99)
      self.handle_action(case_id, reader); 
  };
  ws.onopen = function () {
    var writer = new BufferWriter();
    writer.writeChar(0);
    var my_name = "Hello";
    writer.writeString(my_name);
    ws.send(writer.compile());
    self.my_interval = setInterval(function(player) {
      player.dir += player.adir;
      player.adir += (Math.random()-0.5) / 100;
      player.adir = Math.max(-1.0, Math.min(player.adir, 1.0));  
      if (player.my_id==undefined || player.my_id == -1) return;
      if (player.data[player.my_id].state==0) {
        player.sendMouse(200*Math.sin(player.dir), 200*Math.cos(player.dir));
        if (Math.random() < 1.0/125)
          player.mousedown();
      }
      else if (player.data[player.my_id].state==1) {
        player.sendMouse(200*Math.sin(player.dir), 200*Math.cos(player.dir));
        if (Math.random() < 1.0/50)
          player.mouseup();
      }
    }, 40, self);
  };
  ws.onclose = function() {};
  ws.onerror = function(event) {};
  this.ws = ws;
};
Bot.prototype.connected = function() {
  return this.ws && this.ws.readyState==WebSocket.OPEN;
};
Bot.prototype.mouseup = function() {
  if (this.connected()) {
    var writer = new BufferWriter();
    writer.writeChar(3);
    this.ws.send(writer.compile());
  }
};
Bot.prototype.mousedown = function() {
  if (this.connected()) {
    var writer = new BufferWriter();
    writer.writeChar(2);
    this.ws.send(writer.compile());
  }
};
Bot.prototype.sendMouse = function(x, y) {
  if (this.connected()) {
    var writer = new BufferWriter();
    // console.log(x,y);
    writer.writeChar(1);
    writer.writeFloat(x);
    writer.writeFloat(y);
    this.ws.send(writer.compile());
  }
};
Bot.prototype.keyPressed = function(keyCode) {
  if (this.connected) {
    switch(keyCode) {
      case 48: // 0
        break;
      case 49: // 1
        this.operation = 1;
        break;
      case 50: // 2
        this.operation = 2;
        break;
      case 51: // enable mouse
        this.operation = 3;
        break;
      case 52: // 4
        break;
      case 32: // SPACE
        var writer = new BufferWriter();
        writer.writeChar(4);
        var p = getMouse();
        writer.writeFloat(p.x);
        writer.writeFloat(p.y);
        this.ws.send(writer.compile());
        break;
    }
  }
};

Bot.prototype.handle_action = function(case_id, reader) {
  var id, type_id, shape_id;
  switch (case_id) {
    case 1:   // showUnit
      id = reader.readInt();
      if (!this.data[id]) this.data[id] = {}
      this.data[id].type = reader.readChar();
      shape_id = reader.readChar();
      if (!this.units[id]) {
        if (shape_id==0)
          this.units[id] = new Circle(id);
        else if (shape_id==1)
          this.units[id] = new Segment(id); 
      }
      this.units[id].showUnit(reader);
      break;
    case 2:   // moveUnit
      id = reader.readInt();
      this.units[id].moveUnit(reader);
      break;
    case 3:   // createUnit
      id = reader.readInt();
      break;
    case 4:   // removePlayer
      id = reader.readInt();
      if (id==this.my_id) {
        console.log("You get killed.")
        clearInterval(this.my_interval);
       --bot_count;
        if (bot_count==BOT_NUM-1) {
          new Bot();
        }
        this.my_id = -1;
        this.operation = 99;
      }
      break;
    case 5:   // createPlayer
      //TODO
      this.my_id = reader.readInt();
      console.log("Create player ", this.my_id);
      ++bot_count;
      if (bot_count<BOT_NUM) new Bot();
      this.operation = 1;
      break;
    case 6:   // removeUnit
      id = reader.readInt();
      break;
    case 7:   // changeUnit (Figure)
      id = reader.readInt();
      shape_id = reader.readChar();
      if (shape_id==0)
        this.units[id] = new Circle(id);
      else if (shape_id==1)
        this.units[id] = new Segment(id); 
      this.units[id].showUnit(reader);
      break;
    case 8:   // changeUnit (R)
      id = reader.readInt();
      this.units[id].setR(reader.readFloat());
      break;
    case 9:   // changeUnit (XY)
      id = reader.readInt();
      this.units[id].receiveXY(reader);
      break;
    case 10:  // hideUnit
      id = reader.readInt();
      delete this.units[id];
      break;
    case 11:  // information completion
      id = reader.readInt();
      var old_priv = reader.readChar(), new_priv = reader.readChar(), data_type = reader.readChar();
      // console.log(id, old_priv, new_priv, "information", data_type);
      if (!this.data[id])
        this.data[id] = {type: data_type};
      if (id % 2 == 0) {
        for (var priv = old_priv+1; priv <= new_priv; priv += 1) {
          if (priv==1 && (data_type==0 || data_type==1 || data_type==2))
            this.data[id].owner = reader.readInt();
          if (priv==1 && data_type==4)
            this.data[id].food_type = reader.readChar();
        }
      }
      else {
        for (var priv = old_priv+1; priv <= new_priv; priv += 1) {
          if (priv==1) {
            this.data[id].state = reader.readChar();
            this.data[id].nickname = reader.readString();
          }
          if (priv==2) {
            this.data[id].my_body = reader.readInt();
            this.data[id].my_bullet = reader.readInt();
            this.data[id].my_recall = reader.readInt();
          }
        }
      }
      break;
    case 101:   // fire bullet
      id = reader.readInt();
      this.data[id].state = 1;
      this.data[id].my_bullet = reader.readInt();
      break;
    case 102:   // recall bullet
      id = reader.readInt();
      this.data[id].state = 2;
      this.data[id].my_recall = reader.readInt();
      break;
    case 103:   // change state
      id = reader.readInt();
      this.data[id].state = 1;
      break;
    case 104:   // rebody
      id = reader.readInt();
      if (id==this.my_id)
        this.data[id].my_recall = -1;
      this.data[id].state = 0;
      break;
    case 201:   // debug
      var tick_now = reader.readFloat();
      var total_unit_amount = reader.readInt();
      var total_player_amount = reader.readInt();
      var ttt = reader.readInt();
      var ccc = reader.readInt();
      this.debug_info = (tick_now - this.current_tick_time) + ", unit amout = " + (total_unit_amount)
                  + ", player amount = " + (total_player_amount) + ", total time = " + Math.floor(tick_now/1000);
      this.current_tick_time = tick_now;
      break;
  }
};
//=============================================================================
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
//-----------------------------------------------------------------------------
function Circle(id, x = 0, y = 0, r = 0, vx = 0, vy = 0) {
  Unit.call(this, "CIRCLE", id, x, y, r, vx, vy);
}
Circle.prototype = Object.create(Unit.prototype);
Circle.prototype.resize = function(_x, _y) {
  var xx = _x-this.x, yy = _y-this.y;
  this.r = Math.sqrt(xx*xx+yy*yy);
}
Circle.prototype.showUnit = function(reader) {
  this.x = reader.readFloat();
  this.y = reader.readFloat();
  this.r = reader.readFloat();
}
//-----------------------------------------------------------------------------
function Segment(id, x0 = 0, y0 = 0, x1 = 0, y1 = 0, r = 0, vx = 0, vy = 0) {
  Unit.call(this, "SEGMENT", id, (x0+x1)/2, (y0+y1)/2, r, vx, vy);
  this.dx = x0-this.x;
  this.dy = y0-this.y;
}
Segment.prototype = Object.create(Unit.prototype);
Segment.prototype.resize = function(_x, _y) {
  var x1 = this.x-this.dx, y1 = this.y-this.dy;
  this.moveTo((_x+x1)/2, (_y+y1)/2);
  this.dx = _x-this.x;
  this.dy = _y-this.y;
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
function createBot(num) {
  if (num > 0) {
    new Bot();
    setTimeout(createBot, time_out, num-1);
  }
}
new Bot();