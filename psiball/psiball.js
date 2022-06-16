var canvas = document.getElementById("canvas");
var stage, arena;
var units = [];
var moves = [];
var data = [];
var stream = [];
var latest_tickhead = -10000;
var now = -10000
var now_delta = null;
var my_id = -1;
var DELAY = 50;
var ws;
var ws_address = "ws://45.79.128.170:3000"; // "ws://45.79.152.5:3000"; 
var local_address = "ws://localhost:3000";
var hint, debug_hint, js_hint;
var draft = null;
var test2;
var operation = 999;
var debug_info = "";
var js_info = ""; 
var current_tick_time = 0.0;
var COLORS = ["#F00", "#00F", "#0F0", "#000"];
var paint_time = 0.0, paint_total_time = 0.0, paint_total_times = 0;
var first_paint_time = -1;

function getMouse() {
  return {x: stage.mouseX-canvas.width/2, y: stage.mouseY-canvas.height/2};
}

function Unit(id, shape) {
  this.id = id;
  this.type = data[id].type;
  this.shape = shape;
  this.r = this.d_x = this.d_y = this.x = this.y = 0;;
  this.holder = new createjs.Container();
  arena.addChild(this.holder);
}
Unit.prototype.hide = function() {
  this.holder.removeAllChildren();
  arena.removeChild(this.holder);
}
Unit.prototype.move = function() {
  var left = 0;
  while (left+1<moves[this.id].length && moves[this.id][left+1].time<=now)
    left += 1;
  if (moves[this.id][left].time<latest_tickhead)
    moves[this.id][left].time = latest_tickhead;
  var right = left, ratio = 0;
  if (right+1<moves[this.id].length) {
    right += 1;
    ratio = (now-moves[this.id][left].time)/(moves[this.id][right].time-moves[this.id][left].time);
  }
  this.x = moves[this.id][left].x+ratio*(moves[this.id][right].x-moves[this.id][left].x);
  this.y = moves[this.id][left].y+ratio*(moves[this.id][right].y-moves[this.id][left].y);
  for (var i = 0; i < left; i++)
    moves[this.id].shift();
  this.holder.x = this.x;
  this.holder.y = this.y;
}
Unit.prototype.init = function() {
  this.holder.removeAllChildren();
  return this.holder;
}

function Psiball(id) {
  Unit.call(this, id, 0);
}
Psiball.prototype = Object.create(Unit.prototype);
Psiball.prototype.draw = function() {
  var holder = this.init();
  var color_id = this.type==4 ? data[this.id].food_type : 3;
  var shape = new createjs.Shape();
  shape.graphics.clear().beginFill(COLORS[color_id]).drawCircle(0, 0, this.r);
  shape.alpha = 0.9;
  holder.addChild(shape);
  if (this.type==0 && data[this.id].owner!=-1) {
    var label = new createjs.Text(data[data[this.id].owner].nickname);
    holder.addChild(label);  
  }
}

function Wall(id) {
  Unit.call(this, id, 1);
}
Wall.prototype = Object.create(Unit.prototype);
Wall.prototype.draw = function() {
  var holder = this.init();
  var shape = new createjs.Shape();
  shape.graphics.clear().setStrokeStyle(this.r*2, "round", "round").beginStroke("#000")
    .moveTo(this.d_x, this.d_y)
    .lineTo(-this.d_x, -this.d_y); 
  holder.addChild(shape);
}

function newUnit(unit_id, shape) {
  if (shape==0)
    return new Psiball(unit_id);
  else if (shape==1)
    return new Wall(unit_id);
}

function handle_movelist(timestamp, reader) {
  var id = reader.readInt();
  if (!moves[id])
    moves[id] = [];
  moves[id].push({time: timestamp, x: reader.readFloat(), y: reader.readFloat()});
}

function handle_action(timestamp, reader) {
  var case_id = reader.readChar();
  var id;
  switch (case_id) {
    case 1:   // showUnit
      id = reader.readInt();
      var shape = reader.readChar();
      units[id] = newUnit(id, shape);
      if (shape==0) {
        units[id].r = reader.readFloat();
      }
      else if (units[id].shape==1) {
        units[id].r = reader.readFloat();
        units[id].d_x = reader.readFloat();
        units[id].d_y = reader.readFloat(); 
      }
      units[id].draw();
      break;
    case 2:   // moveUnit
      console.log("IMPOSSIBLE: case 2");
      break;
    case 3:   // createUnit
      id = reader.readInt();
      break;
    case 4:   // removePlayer
      id = reader.readInt();
      if (id==my_id) {
        console.log("You get killed.")
        my_id = -1;
        operation = 99;
      }
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
      units[id].shape = reader.readChar();
      if (units[id].shape==0) {
        units[id].r = reader.readFloat();
      }
      else if (units[id].shape==1) {
        units[id].r = reader.readFloat();
        units[id].d_x = reader.readFloat();
        units[id].d_y = reader.readFloat(); 
      }
      units[id].draw();
      break;
    case 8:   // changeUnit (R)
      id = reader.readInt();
      units[id].r = reader.readFloat();
      units[id].draw();
      break;
    case 9:   // changeUnit (XY)
      console.log("IMPOSSIBLE: case 9")
      break;
    case 10:  // hideUnit
      id = reader.readInt();
      units[id].hide();
      delete units[id];
      break;
    case 11:  // information completion
      id = reader.readInt();
      var old_priv = reader.readChar(), new_priv = reader.readChar(), data_type = reader.readChar();
      if (!data[id])
        data[id] = {type: data_type};
      if (id % 2 == 0) {
        for (var priv = old_priv+1; priv <= new_priv; priv += 1) {
          if (priv==1 && (data_type==0 || data_type==1 || data_type==2))
            data[id].owner = reader.readInt();
          if (priv==1 && data_type==4)
            data[id].food_type = reader.readChar();
        }
      }
      else {
        for (var priv = old_priv+1; priv <= new_priv; priv += 1) {
          if (priv==1) {
            data[id].state = reader.readChar();
            data[id].nickname = reader.readString();
          }
          if (priv==2) {
            data[id].my_body = reader.readInt();
            data[id].my_bullet = reader.readInt();
            data[id].my_recall = reader.readInt();
          }
        }
      }
      break;
    case 101:   // fire bullet
      id = reader.readInt();
      data[id].state = 1;
      data[id].my_bullet = reader.readInt();
      break;
    case 102:   // recall bullet
      id = reader.readInt();
      data[id].state = 2;
      data[id].my_recall = reader.readInt();
      break;
    case 103:   // change state
      id = reader.readInt();
      data[id].state = 1;
      break;
    case 104:   // rebody
      id = reader.readInt();
      if (id==my_id)
        data[id].my_recall = -1;
      data[id].state = 0;
      break;
    case 105:   // gain nickname
      id = reader.readInt();
      data[id].owner = reader.readInt();
      units[id].draw();
      break;
    case 201:   // debug
      var tick_now = reader.readFloat();
      var total_unit_amount = reader.readInt();
      var total_player_amount = reader.readInt();
      var current_player_amount = reader.readInt();
      var connection_count = reader.readInt();
      debug_info = (tick_now - current_tick_time) + ", unit amout = " + (total_unit_amount)
                  + ", player amount = " + (total_player_amount) + ", total time = " + Math.floor(tick_now/1000) + ", current player amount = " + current_player_amount
                  +", connection amount = " + connection_count;
      current_tick_time = tick_now;
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
    var tickhead = reader.readFloat();
    if (now_delta==null)
      now_delta = tickhead-Date.now()-DELAY;
    var timestamp;
    while ((timestamp = reader.readChar())!=255)
      handle_movelist(timestamp+tickhead, reader);
    stream.push({tickhead: tickhead, reader: reader}); 
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

function debug_tick() {
  if (first_paint_time == -1) {
    if (my_id!=-1) {
      first_paint_time = Date.now();
      paint_total_times = 0;
    }
  }
  else {
    paint_total_time = Date.now() - first_paint_time;
    paint_total_times += 1;
    js_info = "Avarage paint tick = " + Math.floor(paint_total_time/paint_total_times);
  }
  hint.text = "Operation = "+operation;
  debug_hint.text = "tick time = "+debug_info;
  js_hint.text = js_info;
}

function tick() { 
  debug_tick();
  if (now_delta==null) 
    return;
  now = Date.now() + now_delta;
  var reader = null, tickhead;
  while (true) {
    if (reader==null) {
      if (stream.length==0)
        break;
      reader = stream[0].reader;
      tickhead = stream[0].tickhead;
      if (now<tickhead)
        break;
      latest_tickhead = tickhead;
      continue;
    }
    var timestamp = reader.peekChar();
    if (timestamp==255) {
      stream.shift();
      reader = null;
      continue;
    }
    if (now<timestamp+tickhead)
      break;
    handle_action(reader.readChar()+tickhead, reader);
  }
  for (var i in units)
    units[i].move();
  if (my_id!=-1) {
    var unit_id = data[my_id].my_body;
    if (data[my_id].state==1)
      unit_id = data[my_id].my_bullet;
    if (data[my_id].state==2)
      unit_id = data[my_id].my_recall;
    arena.x = canvas.width/2-units[unit_id].x;
    arena.y = canvas.height/2-units[unit_id].y; 
    var writer = new BufferWriter();
    writer.writeChar(1);
    var my_mouse = getMouse();
    writer.writeFloat(my_mouse.x);
    writer.writeFloat(my_mouse.y);
    ws.send(writer.compile());
  }
  stage.update();
}


function start() {
  document.getElementById("login").style.display = "none";
  setup_socket();
  stage.addEventListener("stagemousedown", function (evt) {
    var writer = new BufferWriter();
    writer.writeChar(2);
    ws.send(writer.compile());
  });

  stage.addEventListener("stagemouseup", function (evt) {
    var writer = new BufferWriter();
    writer.writeChar(3);
    ws.send(writer.compile());
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
  stage.addChild(hint = new createjs.Text("Operation = "+operation, "bold 20px Arial"));
  stage.addChild(debug_hint = new createjs.Text("tick time = "+debug_info, "bold 20px Arial"));
  stage.addChild(js_hint = new createjs.Text("js info", "bold 20px Arial"));
  hint.x = hint.y = 10;
  debug_hint.x = 10;
  debug_hint.y = 40;
  js_hint.x = 10;
  js_hint.y = 70;
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

      break;
    case 32: // SPACE
      var writer = new BufferWriter();
      writer.writeChar(4);
      var p = getMouse();
      writer.writeFloat(p.x);
      writer.writeFloat(p.y);
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
BufferReader.prototype.peekChar = function () {
  return this.data.getUint8(this.offset, true);
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