#define ALLEGRO_STATICLINK
#include <stdio.h>
#include <math.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

const float FPS=60.0;
const int SCREEN_W=1000;
const int SCREEN_H=900;

enum keycodes
{KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT};

enum tile
{WALL, FLOOR, PLAYER, ENEMY};

void draw_map(enum tile map[50][50]);
void init_map(enum tile map[50][50]);

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

	if(!init(&display, &event_queue, &timer))
	{
		return 1;
	}

	init_map(map);

	while(42)
	{
		ALLEGRO_EVENT ev;
		al_wait_for_event(event_queue, &ev);

		if(ev.type == ALLEGRO_EVENT_TIMER)
		{
			redraw=true;
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
			al_flip_display();
		}
	}

	return 0;
}

void draw_map(enum tile map[50][50])
{
	int x, y;
	ALLEGRO_COLOR colour;

	for(x=0; x<50; x++)
	{
		for(y=0; y<50; y++)
		{
			switch(map[x][y])
			{
				case PLAYER:
					colour=al_map_rgb(0, 255, 0);
					break;
				case ENEMY:
					colour=al_map_rgb(255, 0, 0);
					break;
				case WALL:
					colour=al_map_rgb(0, 0, 0);
					break;
				default:
					colour=al_map_rgb(255, 255, 255);
			}
			al_draw_filled_rectangle(x*18, y*18, x*18+18, y*18+18, colour);
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
