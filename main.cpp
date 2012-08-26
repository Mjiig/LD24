#define ALLEGRO_STATICLINK
#include <cstdio>
#include <cmath>
#include <queue>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

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
	int max_health;
	int speed;
	int x;
	int y;
	int last_move;
	int turns_missed;
	int kills;
	int upgrades;
};

struct enemy
{
	int power;
	int health;
	int max_health;
	int speed;
	int x;
	int y;
	bool exists;
	int turns_missed;
};

void draw_map(enum tile map[50][50]);
void init_map(enum tile map[50][50]);
void init_enemies(struct enemy enemies[100], enum tile map[50][50], int n);
void draw_enemies(struct enemy enemie[100]);
int select_mover(struct player player, struct enemy enemies[100]);
bool move_player(struct player *player, bool keys[4], enum tile map[50][50], struct enemy enemies[100]);
void inc_turns(struct player *player, struct enemy enemies[100]);
void search(int x, int y, enum tile map[50][50], int target_x, int target_y, int * ret_x, int * ret_y);
int is_enemy(int x, int y, struct enemy enemies[100]);
bool is_player(int x, int y, struct player player);
void fight(struct player *player, struct enemy enemies[100], int index);
void breed(struct enemy enemies[100], enum tile map[50][50]);
bool all_enemies_dead(struct enemy enemies[100]);

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
		return 0;
	}

	al_init_font_addon();
	al_init_ttf_addon();

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
	ALLEGRO_FONT *font=NULL;
	bool keys[4]={false, false, false, false};
	bool redraw;
	enum tile map[50][50];
	struct player player;
	struct enemy enemies[100];
	int next;
	int selected_enemy=100;
	int level=1;
	int best=1;

	
	player.power=5;
	player.health=30;
	player.max_health=30;
	player.speed=2;
	player.x=0;
	player.y=0;
	player.turns_missed=0;
	player.kills=0;
	player.upgrades=4;

	if(!init(&display, &event_queue, &timer))
	{
		return 1;
	}

	init_map(map);

	init_enemies(enemies, map, 5);

	font=al_load_font("Xolonium-Regular.otf", 30, 0);

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
				if(move_player(&player, keys, map, enemies))
				{
					player.turns_missed=0;
					inc_turns(&player, enemies);
				}
			}
			else
			{
				int ret_x, ret_y;
				search(enemies[next].x, enemies[next].y, map, player.x, player.y, &ret_x, &ret_y);
				if(!is_player(ret_x, ret_y, player) && is_enemy(ret_x, ret_y, enemies)==100)
				{
					enemies[next].x=ret_x;
					enemies[next].y=ret_y;
				}
				if(is_player(ret_x, ret_y, player))
				{
					fight(&player, enemies, next);
				}
				enemies[next].turns_missed=0;
				inc_turns(&player, enemies);
				if(!(rand()%(150/level)))
				{
					breed(enemies, map);
				}
			}

			if(enemies[selected_enemy].health<=0)
			{
				selected_enemy=100;
			}

			if(all_enemies_dead(enemies))
			{
				level++;
				init_map(map);
				init_enemies(enemies, map, level*5);
				player.x=0;
				player.y=0;
				if(level>best)
				{
					best=level;
				}
			}

			if(player.health<=0)
			{
				level=1;
				init_map(map);
				init_enemies(enemies, map, level*5);
				player.power=5;
				player.health=30;
				player.max_health=30;
				player.speed=2;
				player.x=0;
				player.y=0;
				player.turns_missed=0;
				player.kills=0;
				player.upgrades=0;
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
		else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
		{
			int old_selected=selected_enemy;

			selected_enemy=is_enemy(ev.mouse.x/18, ev.mouse.y/18, enemies);

			if(selected_enemy==100)
				selected_enemy=old_selected;

			if(ev.mouse.x>900 && player.kills >= (player.upgrades * player.upgrades)/10)
			{
				bool change=false;
				if(ev.mouse.y>10 && ev.mouse.y<40)
				{
					player.max_health+=5;
					player.health=player.max_health;
					change=true;
				}
				else if(ev.mouse.y>50 && ev.mouse.y<80)
				{
					player.power+=2;
					change=true;
				}
				else if(ev.mouse.y>90 && ev.mouse.y<120)
				{
					player.speed+=1;
					change=true;
				}
				
				if(change)
				{
					player.upgrades+=1;
				}
			}
		}

		if(redraw && al_is_event_queue_empty(event_queue))
		{
			redraw=false;
			al_clear_to_color(al_map_rgb(255, 255, 255));
			draw_map(map);
			draw_enemies(enemies);
			al_draw_bitmap(player_img, player.x*18, player.y*18, 0);
			al_draw_textf(font, al_map_rgb(0, 255, 0), 950, 10, ALLEGRO_ALIGN_CENTRE, "%d", player.health);
			al_draw_textf(font, al_map_rgb(0, 255, 0), 950, 50, ALLEGRO_ALIGN_CENTRE, "%d", player.power);
			al_draw_textf(font, al_map_rgb(0, 255, 0), 950, 90, ALLEGRO_ALIGN_CENTRE, "%d", player.speed);
			al_draw_textf(font, al_map_rgb(0, 0, 255), 950, 150, ALLEGRO_ALIGN_CENTRE, "%d", player.kills);
			al_draw_textf(font, al_map_rgb(0, 0, 255), 950, 190, ALLEGRO_ALIGN_CENTRE, "%d", (player.upgrades*player.upgrades)/10);
			if(selected_enemy!=100)
			{
				al_draw_textf(font, al_map_rgb(255, 0, 0), 950, 250, ALLEGRO_ALIGN_CENTRE, "%d", enemies[selected_enemy].health);
				al_draw_textf(font, al_map_rgb(255, 0, 0), 950, 290, ALLEGRO_ALIGN_CENTRE, "%d", enemies[selected_enemy].power);
				al_draw_textf(font, al_map_rgb(255, 0, 0), 950, 330, ALLEGRO_ALIGN_CENTRE, "%d", enemies[selected_enemy].speed);
			}
			al_draw_textf(font, al_map_rgb(0, 0, 0), 950, 390, ALLEGRO_ALIGN_CENTRE, "%d", level);
			al_draw_textf(font, al_map_rgb(0, 0, 0), 950, 430, ALLEGRO_ALIGN_CENTRE, "%d", best);
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

void init_enemies(struct enemy enemies[100], enum tile map[50][50], int n)
{
	int i;

	if(n>100)
	{
		n=100;
	}

	for(i=0; i<n; i++)
	{
		enemies[i].power=1;
		enemies[i].health=10;
		enemies[i].speed=1;
		enemies[i].x=rand()%40+10;
		enemies[i].y=rand()%40+10;
		enemies[i].exists=true;
		enemies[i].turns_missed=0;
		enemies[i].max_health=5;
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

bool move_player(struct player *player, bool keys[4], enum tile map[50][50], struct enemy enemies[100])
{
	int new_x=player->x;
	int new_y=player->y;
	if(player->last_move>1)
	{
		if(keys[KEY_UP] && player->y>0 && map[player->x][player->y-1]==FLOOR)
			new_y--;
		else if(keys[KEY_RIGHT] && player->x<49 && map[player->x+1][player->y]==FLOOR)
			new_x++;
		else if(keys[KEY_DOWN] && player->y<49 && map[player->x][player->y+1]==FLOOR)
			new_y++;
		else if(keys[KEY_LEFT] && player->x>0 && map[player->x-1][player->y]==FLOOR)
			new_x--;

		if(is_enemy(new_x, new_y, enemies)==100)
		{
			player->x=new_x;
			player->y=new_y;
		}
		else
		{
			fight(player, enemies, is_enemy(new_x, new_y, enemies));
		}

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

	while(!x_queue.empty())
	{
		int current_x=x_queue.front();
		int current_y=y_queue.front();

		while(x_queue.size() == 0 || y_queue.size()==0);

		x_queue.pop();
		y_queue.pop();

		if(selected[current_x][current_y] || map[current_x][current_y]==WALL || dist[current_x][current_y]>25)
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

	*ret_x=x;
	*ret_y=y;
}

bool is_player(int x, int y, struct player player)
{
	if(x==player.x && y==player.y)
	{
		return true;
	}
	return false;
}

int is_enemy(int x, int y, struct enemy enemies[100])
{
	for(int i=0; i<100; i++)
	{
		if(enemies[i].exists && enemies[i].x==x && enemies[i].y==y)
		{
			return i;
		}
	}
	return 100; //stupid sentinal value
}

void fight(struct player *player, struct enemy enemies[100], int index)
{
	int hit_p=rand()%(player->power+1);
	int hit_e=rand()%(enemies[index].power+1);

	player->health-=hit_e;
	enemies[index].health-=hit_p;
	if(enemies[index].health<=0)
	{
		enemies[index].exists=false;
		player->kills++;
	}
}

void breed(struct enemy enemies[100], enum tile map[50][50])
{
	//Pick an enemy at random
	int parent;
	int accumulator=1;
	for(int i=0; i<100; i++)
	{
		if(enemies[i].exists)
		{
			if(!(rand()%accumulator))
			{
				parent=i;
			}
			accumulator++;
		}
	}

	int xs[]={enemies[parent].x+1, enemies[parent].x-1, enemies[parent].x, enemies[parent].x};
	int ys[]={enemies[parent].y, enemies[parent].y, enemies[parent].y+1, enemies[parent].y-1};

	int child_x;
	int child_y;

	accumulator=0;

	for(int c=0; c<4; c++)
	{
		if(xs[c]<0 || xs[c]>49 || ys[c]<0 || ys[c]>49 || map[xs[c]][ys[c]]==WALL || is_enemy(xs[c], ys[c], enemies)!=100)
		{
			accumulator++;
			continue;
		}
		child_x=xs[c];
		child_y=ys[c];
	}

	if(accumulator==4)
	{
		return;
	}


	int child_index;


	for(child_index=0; child_index<100; child_index++)
	{
		if(!(enemies[child_index].exists))
		{
			break;
		}
	}

	if(child_index==100)
	{
		return;
	}

	enemies[child_index].x=child_x;
	enemies[child_index].y=child_y;
	enemies[child_index].exists=true;
	enemies[child_index].turns_missed=0;
	enemies[child_index].power=enemies[parent].power+(rand()%3 ? 0 : (1));
	enemies[child_index].speed=enemies[parent].speed+(rand()%3 ? 0 : 1);
	enemies[child_index].max_health=enemies[parent].max_health+(rand()%3 ? 0 : 5);
	enemies[child_index].health=enemies[child_index].max_health;
}

bool all_enemies_dead(struct enemy enemies[100])
{
	for(int i=0; i<100; i++)
	{
		if(enemies[i].exists)
		{
			return false;
		}
	}
	return true;
}
