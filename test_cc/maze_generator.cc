#include "maze.h"
#include "../lib/io/SocketManager.h"
#include "../lib/util/Figure.h"
#include "../lib/util/locate.h"
#include "../lib/engine/Game.h"

Game* g;
const double len_x = 50, len_y = 35;


enum Unit::Type: uint8_t {
  TEST, WALL
};

struct TestUnit: Unit {
  TestUnit(): Unit(Unit::TEST, 1) {
    v = XY(0, 0);
    mass = 1;  
  }
  void toMessage(uint8_t priv, Socket& socket) override {
  }
};

struct TestWall: Unit {
  TestWall(): Unit(Unit::WALL, 1) {
    v = XY(0, 0);
    mass = INFINITY_MASS;
  }
  void toMessage(uint8_t priv, Socket& socket) override {}
};

enum Player::Type: uint8_t {
  TESTER
};

struct TestPlayer : Player { 
  TestPlayer(shared_ptr<Socket>& socket_) : Player(TESTER, socket_, 1) {}
};

void handle_start(Start* start) {
  auto player = new TestPlayer(start->socket);
  g->createPlayer(player);
}


void handle_message(Command* cmd) {
  auto& reader = *(cmd->reader);
  uint8_t case_id;
  reader >> case_id;
  double x, y, r, dir, x2, y2;
  switch (case_id) {
    case 1:
    {
      reader >> x >> y >> r >> dir;
      auto unit = new TestUnit();
      auto figure = new Figure::Circle(XY(x,y), r);
      g->createUnit(unit, figure);
      break;
    }
    case 2:
    {
      reader >> x >> y >> x2 >> y2 >> r;
      auto unit = new TestWall();
      auto figure = new Figure::Segment(XY(x,y), XY(x2,y2), r);
      g->createUnit(unit, figure);
      break;
    }
    case 3:
    {
      uint32_t id = 0;
      g->unit_list.foreach([&id](uint32_t unit_id) {
        id = max(id, unit_id);
      });
      auto me = g->unit_list.get(id);
      auto unit = new TestUnit();
      unit->v = me->v*16;
      unit->mass = 2;
      auto figure = new Figure::Circle(g->getXY(id), g->getR(id)/4);
      g->createUnit(unit, figure);
      break;
    }
    case 4:
    {
      reader >> x >> y;
      XY p(x, y);
      g->unit_list.foreach([&p](uint32_t unit_id) {
        g->changeXY(unit_id, p);
      });
    }
    case 5:
    {
      reader >> x >> y;
      XY p(x, y);
      g->unit_list.foreach([&p](uint32_t unit_id) {
        if (g->getFigure(unit_id)->type==Figure::CIRCLE) {
          double p_len = p.norm();
          double max_len = 100;
          double min_len = 15;
          double speed = 0.2;
          if (p_len < min_len)
            p_len = 0;
          else if (p_len < max_len)
            p_len = (p_len-min_len)/(max_len-min_len)*speed;
          else
            p_len = speed;
          g->changeV(unit_id, p.normalized()*p_len);
        }
      });
    }
  }
}

void handle_close(Close* close) {}

void handle_init(Init* init) {

    int matrix_dim = 40, i;
    
    stack = (int**)calloc(matrix_dim*matrix_dim * 100, sizeof(int*)); //Stack olusturma
    for (i = 0; i < matrix_dim*matrix_dim * 100; i++) {
        stack[i] = (int*)calloc(2, sizeof(int));
    }

    neighbors_matrix = (int**)calloc(4, sizeof(int*));
    for (i = 0; i < 4; i++) {
        neighbors_matrix[i] = (int*)calloc(3, sizeof(int));
    }
    neighbors_matrix[0][0] = -1;
    neighbors_matrix[0][1] = 0;

    neighbors_matrix[1][0] = 0;
    neighbors_matrix[1][1] = 1;

    neighbors_matrix[2][0] = 1;
    neighbors_matrix[2][1] = 0;

    neighbors_matrix[3][0] = 0;
    neighbors_matrix[3][1] = -1;

    matrix_dim += 2;
    maze_dim = 2*(matrix_dim - 2) + 1;

    int **maze = (int**)calloc(matrix_dim, sizeof(int*)); 
    for (i = 0; i < matrix_dim; i++) {
        maze[i] = (int*)calloc(matrix_dim, sizeof(int));
    }

    int **visited = (int**)calloc(matrix_dim, sizeof(int*)); 
    for (i = 0; i < matrix_dim; i++) {
        visited[i] = (int*)calloc(matrix_dim, sizeof(int));
    }

    //freopen("maze.txt", "w", stdout);
    //printf("haha\n");
    char** ans = generate_maze(maze, 1, 1, visited, matrix_dim); 
    for (int i = 0; i < maze_dim; i++) {
        for (int j = 0; j < maze_dim; j++) {
            //printf(" %c ", ans[i][j]);
          if (ans[i][j]=='-') {
            double x0 = (j-1)*len_x, x1 = (j+1)*len_x, y0 = i*len_y, y1 = y0;
            g->createUnit(new TestWall(), 
              new Figure::Segment(XY(x0, y0), XY(x1, y1), 5));
          }
          if (ans[i][j]=='|') {
            double x0 = j*len_x, x1 = x0, y0 = (i-1)*len_y, y1 = (i+1)*len_y;
            g->createUnit(new TestWall(), 
              new Figure::Segment(XY(x0, y0), XY(x1, y1), 5));
          }
        }
//        printf("\n");
    }
}

void handle_create_player(CreatePlayer* cp) {
  g->createUnit(new TestUnit(), new Figure::Circle(XY(len_x/2,len_y/2), 10));
}

void handle_collision(OnCollision* oc) {
  uint32_t id1 = oc->id1, id2 = oc->id2;
  if (g->getFigure(id1)->type==Figure::SEGMENT) {
    uint32_t id3 = id1;
    id1 = id2;
    id2 = id3;
  }
  if (g->getFigure(id1)->type!=Figure::CIRCLE || g->getFigure(id2)->type!=Figure::SEGMENT)
    return;
  g->changeXY(id1, XY(len_x/2, len_y/2));
}

int main(void)
{
  g = new Game();
  g->getCollisionType = [](uint32_t id1, uint32_t id2){
    if (g->unit_list.get(id1)->type == Unit::WALL && g->unit_list.get(id2)->type == Unit::WALL) 
      return CollisionType::NONE;
    return CollisionType::REPULSIVE_DETECTABLE;
  };
  g->trigger(handle_init);
  g->trigger(handle_start);
  g->trigger(handle_create_player);
  g->trigger(handle_message);
  g->trigger(handle_close);
  // g->trigger(handle_collision);
  g->mainLoop();
  return 0;
}

 