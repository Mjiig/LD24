#define ALLEGRO_STATICLINK
#include <cstdio>
#include <cmath>
#include <queue>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

const float FPS=60.0;
const int SCREEN_W=1000;
const int SCREEN_H=900;

ALLEGRO_BITMAP *wall_img;
ALLEGRO_BITMAP *floor_img;
ALLEGRO_BITMAP *player_img;
ALLEGRO_BITMAP *enemy_img;

enum keycodes
{KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT};

enum tile
{WALL, FLOOR};

struct player
{
	int power;
	int health;
	int speed;
	int x;
	int y;
	int last_move;
	int turns_missed;
};

struct enemy
{
	int power;
	int health;
	int speed;
	int x;
	int y;
	int last_breed;
	bool exists;
	int turns_missed;
};

void draw_map(enum tile map[50][50]);
void init_map(enum tile map[50][50]);
void init_enemies(struct enemy enemies[100], enum tile map[50][50]);
void draw_enemies(struct enemy enemie[100]);
int select_mover(struct player player, struct enemy enemies[100]);
bool move_player(struct player *player, bool keys[4], enum tile map[50][50]);
void inc_turns(struct player *player, struct enemy enemies[100]);
void search(int x, int y, enum tile map[50][50], int target_x, int target_y, int * ret_x, int * ret_y);

int init(ALLEGRO_DISPLAY ** display, ALLEGRO_EVENT_QUEUE ** event_queue, ALLEGRO_TIMER ** timer)
{

	*display = NULL;
	*event_queue=NULL;
	*timer=NULL;

	if(!al_init())
	{
		fprintf(stderr, "Failed to initialise allegro!\n");
		return 0;
	}

	*timer=al_create_timer(1.0/FPS);
	if(!*timer)
	{
		fprintf(stderr, "Failed to create timer!\n");
		return 0;
	}

	*display=al_create_display(SCREEN_W, SCREEN_H);
	if(!*display)
	{
		fprintf(stderr, "Failed to create display\n");
		return 0;
	}

	if(!al_init_primitives_addon())
	{
		fprintf(stderr, "Failed to initialise primitives!\n");
		return 0;
	}

	if(!al_init_image_addon())
	{
		fprintf(stderr, "Failed to initialise image addon\n");
		return 0;
	}

	if(!al_install_mouse())
	{
		fprintf(stderr, "Could not initialise mouse bindings!\n");
		return 0;
	}

	if(!al_install_keyboard())
	{
		fprintf(stderr, "Could not initilaise keyboard bindings!\n");
	}

	*event_queue=al_create_event_queue();
	if(!*event_queue)
	{
		fprintf(stderr, "Failed to create event queue!\n");
		return 0;
	}

	al_register_event_source(*event_queue, al_get_display_event_source(*display));
	al_register_event_source(*event_queue, al_get_timer_event_source(*timer));
	al_register_event_source(*event_queue, al_get_mouse_event_source());
	al_register_event_source(*event_queue, al_get_keyboard_event_source());

	al_clear_to_color(al_map_rgb(255,255,255));

	al_flip_display();
	
	al_start_timer(*timer);
	
	//initialise some horrible global constants!
	wall_img=al_load_bitmap("wall.png");
	floor_img=al_load_bitmap("floor.png");
	player_img=al_load_bitmap("player.png");
	enemy_img=al_load_bitmap("enemy.png");

	return 1;
}

int main()
{
	ALLEGRO_DISPLAY *display = NULL;
	ALLEGRO_EVENT_QUEUE *event_queue=NULL;
	ALLEGRO_TIMER *timer=NULL;
	bool keys[4]={false, false, false, false};
	bool redraw;
	enum tile map[50][50];
	struct player player;
	struct enemy enemies[100];
	int next;

	
	player.power=1;
	player.health=10;
	player.speed=2;
	player.x=0;
	player.y=0;
	player.turns_missed=0;

	if(!init(&display, &event_queue, &timer))
	{
		return 1;
	}

	init_map(map);

	init_enemies(enemies, map);

	while(42)
	{
		ALLEGRO_EVENT ev;
		al_wait_for_event(event_queue, &ev);

		if(ev.type == ALLEGRO_EVENT_TIMER)
		{
			redraw=true;
			next=select_mover(player, enemies);
			if(next==100)
			{
				if(move_player(&player, keys, map))
				{
					player.turns_missed=0;
					inc_turns(&player, enemies);
				}
			}
			else
			{
				int ret_x, ret_y;
				search(enemies[next].x, enemies[next].y, map, player.x, player.y, &ret_x, &ret_y);
				enemies[next].x=ret_x;
				enemies[next].y=ret_y;
				enemies[next].turns_missed=0;
				inc_turns(&player, enemies);
			}

		}
		else if(ev.type==ALLEGRO_EVENT_DISPLAY_CLOSE)
		{
			break;
		}
		else if(ev.type == ALLEGRO_EVENT_KEY_DOWN)
		{
			switch(ev.keyboard.keycode)
			{
				case ALLEGRO_KEY_UP:
					keys[KEY_UP]=true;
					break;
				case ALLEGRO_KEY_DOWN:
					keys[KEY_DOWN]=true;
					break;
				case ALLEGRO_KEY_RIGHT:
					keys[KEY_RIGHT]=true;
					break;
				case ALLEGRO_KEY_LEFT:
					keys[KEY_LEFT]=true;
					break;
			}
		}
		else if(ev.type == ALLEGRO_EVENT_KEY_UP)
		{
			switch(ev.keyboard.keycode)
			{
				case ALLEGRO_KEY_UP:
					keys[KEY_UP]=false;
					break;
				case ALLEGRO_KEY_DOWN:
					keys[KEY_DOWN]=false;
					break;
				case ALLEGRO_KEY_RIGHT:
					keys[KEY_RIGHT]=false;
					break;
				case ALLEGRO_KEY_LEFT:
					keys[KEY_LEFT]=false;
					break;
			}
		}
		if(redraw && al_is_event_queue_empty(event_queue))
		{
			redraw=false;
			al_clear_to_color(al_map_rgb(255, 255, 255));
			draw_map(map);
			draw_enemies(enemies);
			al_draw_bitmap(player_img, player.x*18, player.y*18, 0);
			al_flip_display();
		}
	}

	return 0;
}

void draw_map(enum tile map[50][50])
{
	int x, y;
	ALLEGRO_BITMAP * image;

	for(x=0; x<50; x++)
	{
		for(y=0; y<50; y++)
		{
			switch(map[x][y])
			{
				case WALL:
					image=wall_img;
					break;
				default:
					image=floor_img;
			}
			al_draw_bitmap(image,x*18, y*18, 0);
		}
	}
}

void init_map(enum tile map[50][50])
{
	int x, y;
	int len, width;
	int offset_x, offset_y;
	int x1, y1;
	int direction;

	for(x=0; x<50; x++)
	{
		for(y=0; y<50; y++)
		{
			if(x%49==0 || y%49==-0)
			{
				map[x][y]=FLOOR;
			}
			else
			{
				map[x][y]=WALL;
			}
		}
	}

	for(x=0; x<10; x++)
	{
		for(y=0; y<10; y++)
		{
			if(rand()%2)
			{
				continue;
			}
			len=rand()%4+1;
			width=rand()%4+1;
			offset_y=rand()%(5-len);
			offset_x=rand()%(5-width);

			for(x1=0; x1<width; x1++)
			{
				for(y1=0; y1<len; y1++)
				{
					map[x*5+offset_x+x1+1][y*5+offset_y+y1+1]=FLOOR;
				}
			}

			direction=rand()%4;

			switch(direction)
			{
				case 0: //up
					x1=x*5+offset_x+rand()%width+1;
					y1=y*5+offset_y;
					break;
				case 1: //right
					x1=x*5+offset_x+width+1;
					y1=y*5+offset_y+rand()%len+1;
					break;
				case 2: //down
					x1=x*5+offset_x+rand()%width+1;
					y1=y*5+offset_y+len+1;
					break;
				case 3: //left
					x1=x*5+offset_x;
					y1=y*5+offset_y+rand()%len+1;
					break;
			}

			if(x1==50 || y1==50)
				continue;

			while(map[x1][y1]!=FLOOR)
			{
				map[x1][y1]=FLOOR;
				switch(direction)
				{
					case 0: //up
						x1--;
						break;
					case 1: //right
						y1++;
						break;
					case 2: //down
						x1++;
						break;
					case 3: //left
						y1--;
						break;
				}
			}
		}
	}
}

void init_enemies(struct enemy enemies[100], enum tile map[50][50])
{
	int i;

	for(i=0; i<10; i++)
	{
		enemies[i].power=1;
		enemies[i].health=10;
		enemies[i].speed=1;
		enemies[i].x=rand()%40+10;
		enemies[i].y=rand()%40+10;
		enemies[i].last_breed=0;
		enemies[i].exists=true;
		enemies[i].turns_missed=0;
		while(map[enemies[i].x][enemies[i].y] != FLOOR)
		{
			enemies[i].x=rand()%40+10;
			enemies[i].y=rand()%40+10;
		}
	}

	for(;i<100; i++)
	{
		enemies[i].exists=false;
	}
}

void draw_enemies(struct enemy enemies[100])
{
	int i;

	for(i=0; i<100; i++)
	{
		if(enemies[i].exists)
		{
			al_draw_bitmap(enemy_img, enemies[i].x*18, enemies[i].y*18, 0);
		}
	}
}

int select_mover(struct player player, struct enemy enemies[100])
{
	//return value of 100 means player, anything references the enemies array
	int i;
	int max=player.turns_missed*player.speed;
	int current=100;

	for(i=0; i<100; i++)
	{
		if(enemies[i].exists && enemies[i].speed*enemies[i].turns_missed > max)
		{
			max=enemies[i].speed*enemies[i].turns_missed;
			current=i;
		}
	}
	return current;
}

bool move_player(struct player *player, bool keys[4], enum tile map[50][50])
{
	if(player->last_move>3)
	{
		if(keys[KEY_UP] && player->y>0 && map[player->x][player->y-1]==FLOOR)
			player->y--;
		else if(keys[KEY_RIGHT] && player->x<49 && map[player->x+1][player->y]==FLOOR)
			player->x++;
		else if(keys[KEY_DOWN] && player->y<49 && map[player->x][player->y+1]==FLOOR)
			player->y++;
		else if(keys[KEY_LEFT] && player->x>0 && map[player->x-1][player->y]==FLOOR)
			player->x--;

		if(keys[KEY_UP] || keys[KEY_DOWN] || keys[KEY_RIGHT] || keys[KEY_LEFT])
		{
			player->last_move=0;
			return true;
		}
	}
	else
	{
		player->last_move++;
	}
	return false;
}

void inc_turns(struct player *player, struct enemy enemies[100])
{
	int i;
	player->turns_missed++;
	for(i=0; i<100; i++)
	{
		enemies[i].turns_missed++;
	}
}

void search(int x, int y, enum tile map[50][50], int target_x, int target_y, int * ret_x, int * ret_y)
{
	bool selected[50][50];
	int prev_x[50][50];
	int prev_y[50][50];
	int dist[50][50];

	std::queue<int> x_queue;
	std::queue<int> y_queue;

	for(int x1=0; x1<50; x1++)
	{
		for(int y1=0; y1<50; y1++)
		{
			selected[x1][y1]=false;
			prev_x[x1][y1]=-1;
			prev_y[x1][y1]=-1;
			dist[x1][y1]=1000000;
		}
	}

	dist[x][y]=0;

	x_queue.push(x);
	y_queue.push(y);

	while(true)
	{
		int current_x=x_queue.front();
		int current_y=y_queue.front();

		while(x_queue.size() == 0 || y_queue.size()==0);

		x_queue.pop();
		y_queue.pop();

		if(selected[current_x][current_y] || map[current_x][current_y]==WALL)
			continue;

		selected[current_x][current_y]=true;

		int xs[]={current_x+1, current_x-1, current_x, current_x};
		int ys[]={current_y, current_y, current_y+1, current_y-1};

		for(int c=0; c<4; c++)
		{
			if(xs[c]<0 || xs[c]>49 || ys[c]<0 || ys[c]>49)
				continue;
			if(dist[xs[c]][ys[c]]>dist[current_x][current_y])
			{
				dist[xs[c]][ys[c]]=dist[current_x][current_y]+1;
				prev_x[xs[c]][ys[c]]=current_x;
				prev_y[xs[c]][ys[c]]=current_y;
			}

			x_queue.push(xs[c]);
			y_queue.push(ys[c]);
		}

		if(current_x == target_x && current_y==target_y)
		{
			while(dist[current_x][current_y]>1)
			{
				int temp_x=current_x;
				current_x=prev_x[current_x][current_y];
				current_y=prev_y[temp_x][current_y];
			}

			*ret_x=current_x;
			*ret_y=current_y;
			return;
		}
	}
}