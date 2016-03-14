#include <iostream>
#include <sfml/graphics.hpp>
#include <sfml/opengl.hpp>
#include "greedy_meshing.hpp"
#include  "helper.hpp"


// 10 layers you can use
enum LayerNambers {
	DrawLayer0 = 0, DrawLayer1, DrawLayer2, DrawLayer3, DrawLayer4,
	DrawLayer5, DrawLayer6, DrawLayer7, DrawLayer8, DrawLayer9
};

// define blocks
enum BlockNumbers {
	Air = 0, Water, Dirt, Turf, Stone, Cobblestone, 
	CoalOre, IronOre, GoldOre, DiamondOre, RedstoneOre, Bedrock, 
	LogOak, PlanksOak, LeavesOak, Glass, TNT,
	BlockCount = 17
};

// block texture filenames
const char* block_filename[] = {
	"none", "water", "dirt", "turf", "stone", "cobblestone",
	"coal_ore", "iron_ore", "gold_ore", "diamond_ore", "redstone_ore", "bedrock",
	"log_oak", "planks_oak", "leaves_oak", "glass", "tnt"
};

// winodw size
const int window_width = 1280;
const int window_height = 800;

// map size
const int map_width = 40;
const int map_height = 25;

// block size
const int block_width = window_width / map_width;
const int block_height = window_height / map_height;

// menu
const int block_icon_size = 3;
const int menu_width = 8 * block_icon_size;
const int menu_height =  2 * block_icon_size;

// blocks
int blocks[map_width][map_height];
int dark_blocks[map_width][map_height];
sf::Vector2i select_xy;    // the block which you selected
int dark_blocks_drawing = 0;
int brush_picking = 0;
int brush = Dirt;

// fluid simulation
const float max_mass = 1;
const float min_mass = 0.0001;
const float max_compress = 0.02;
const float max_speed = 1;
const float min_flow = 0.01;
float mass[map_width+2][map_height+2];
float new_mass[map_width+2][map_height+2];

// opengl's vars
GLuint list_block;
GLuint tex_sky, tex_blocks[BlockCount];
int quad_frames = 0;

//greedy meshing
std::vector<Quad> quads;
std::vector<Quad> dark_quads;
int checked[map_width][map_height];

// sfml text
sf::Font font;
sf::Text text_fps, text_selected;


void InitDemo()
{
	// opengl states
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// opengl pipeline setting
	glViewport(0, 0, window_width, window_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, map_width, 0, map_height, -10, 10);

	// load Font
	font.loadFromFile("data/sansation.ttf");
	text_fps.setFont(font);
	text_selected.setFont(font);
	text_selected.setPosition(0, 32);

	// load block textures
	for (int i = 0; i < BlockCount; ++i)
	{
		sf::String path = "data/";
		sf::String filename = block_filename[i];
		if (filename != "none") {
			filename += ".png";
			path += filename;
			tex_blocks[i] = helper::LoadTexture(path, GL_LINEAR, GL_NEAREST);
		}
	}
	// load sky texture
	tex_sky = helper::LoadTexture("data/sky.png", GL_LINEAR, GL_LINEAR);

	// build up a list of unit block
	list_block = glGenLists(1);
	glNewList(list_block, GL_COMPILE);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex3f(0, 0, 0);
	glTexCoord2f(1, 0); glVertex3f(1, 0, 0);
	glTexCoord2f(1, 1); glVertex3f(1, 1, 0);
	glTexCoord2f(0, 1); glVertex3f(0, 1, 0);
	glEnd();
	glEndList();

	// clear the arrays
	for (int x = 0; x < map_width; ++x)
		for (int y = 0; y < map_height; ++y) {
			blocks[x][y] = Air;
			dark_blocks[x][y] = Air;
		}

	for (int x = 0; x < map_width+2; ++x)
		for (int y = 0; y < map_height+2; ++y) {
			mass[x][y] = 0;
			new_mass[x][y] = 0;
		}
}


void UpdateBlockStates(sf::RenderWindow* window)
{
	// get cursor position in window coordinate
	sf::Vector2i cursor = sf::Mouse::getPosition(*window);
	// transform to block coordinate
	select_xy = sf::Vector2i(cursor.x / block_width, (-cursor.y+window_height) / block_height);

	// clip to a valid range
	select_xy.x = helper::Clip(select_xy.x, 0, map_width-1);
	select_xy.y = helper::Clip(select_xy.y, 0, map_height-1);

	// rendering mode
	if (brush_picking == 0) {
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
		{
			if (dark_blocks_drawing == 0) {
				blocks[select_xy.x][select_xy.y] = brush;
				mass[select_xy.x+1][select_xy.y+1] = Air;
				if (brush == Water)
					mass[select_xy.x+1][select_xy.y+1] = max_mass;
				else
					GreedyMeshing(quads, checked, blocks, map_width, map_height);
			}
			else {
				dark_blocks[select_xy.x][select_xy.y] = brush;
				GreedyMeshing(dark_quads, checked, dark_blocks, map_width, map_height);
			}
		}
		else if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
		{
			if (dark_blocks_drawing == 0) {
				blocks[select_xy.x][select_xy.y] = Air;
				mass[select_xy.x+1][select_xy.y+1] = 0;
				GreedyMeshing(quads, checked, blocks, map_width, map_height);
			}
			else {
				dark_blocks[select_xy.x][select_xy.y] = Air;
				GreedyMeshing(dark_quads, checked, dark_blocks, map_width, map_height);
			}
		}
	}
	// pick up mode
	else if (brush_picking == 1) {
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
		{
			float ori_x = (map_width-menu_width) / 2.0f;
			float ori_y = (map_height-menu_height) / 2.0f;
			sf::Vector2i base(ori_x*block_width, ori_y*block_height);
			sf::Vector2i click_xy(cursor.x-base.x, (-cursor.y+window_height)-base.y);

			int px = click_xy.x / (block_width*block_icon_size);
			int py = click_xy.y / (block_height*block_icon_size);
			if (px < 0 || px > BlockCount/2-1)  return;
			if (py < 0 || py > 1)  return;

			if (BlockCount & 1 == 0)
				brush = px + py * (BlockCount/2);
			else
				brush = px + py * (BlockCount/2) + 1;
		}	
	}
}


float GetStableState(float total_mass)
{
	if (total_mass <= 1)
		return 1;
	else if (total_mass < 2.0f*max_mass + max_compress)
		return (max_mass*max_mass + total_mass*max_compress) / (max_mass + max_compress);
	else
		return (total_mass + max_compress) / 2.0f;
}


void SimulateFluid()
{
	float flow = 0;
	float remaining_mass;
	
	for (int x = 1; x <= map_width; ++x)
		for (int y = 1; y <= map_height; ++y)
		{
			// skip solid blocks
			if (blocks[x-1][y-1] > Water)  continue;

			// custom push-only flow
			flow = 0;
			remaining_mass = mass[x][y];
			if (remaining_mass <= 0)  continue;

			// the block below this one
			if (blocks[x-1][y-2] <= Water) {
				flow = GetStableState(remaining_mass + mass[x][y-1]) - mass[x][y-1];
				if (flow > min_flow)  flow *= 0.5;
				flow = helper::Clip(flow, 0.0f, std::min(max_speed, remaining_mass));

				new_mass[x][y] -= flow;
				new_mass[x][y-1] += flow;
				remaining_mass -= flow;
			}

			if (remaining_mass <= 0)  continue;

			// left
			if (blocks[x-2][y-1] <= Water) {
				flow = (mass[x][y] - mass[x-1][y]) /4;
				if (flow > min_flow)  flow *= 0.5;
				flow = helper::Clip(flow, 0.0f, remaining_mass);
			
				new_mass[x][y] -= flow;
				new_mass[x-1][y] += flow;
				remaining_mass -= flow;
			}

			if (remaining_mass <= 0)  continue;

			// right
			if (blocks[x][y-1] <= Water) {
				flow = (mass[x][y] - mass[x+1][y]) /4;
				if (flow > min_flow)  flow *= 0.5;
				flow = helper::Clip(flow, 0.0f, remaining_mass);
			
				new_mass[x][y] -= flow;
				new_mass[x+1][y] += flow;
				remaining_mass -= flow;
			}

			if (remaining_mass <= 0)  continue;

			// up. only compressed water flows upwards
			if (blocks[x-1][y] <= Water) {
				flow = remaining_mass - GetStableState(remaining_mass + mass[x][y+1]);
				if (flow > min_flow)  flow *= 0.5;
				flow = helper::Clip(flow, 0.0f, std::min(max_speed, remaining_mass));
				
				new_mass[x][y] -= flow;
				new_mass[x][y+1] += flow;
				remaining_mass -= flow;
			}
		}

	// copy the new mass values to  the mass array
	for (int x = 0; x < map_width+2; ++x)
		for (int y = 0; y < map_height+2; ++y) {
			mass[x][y] = new_mass[x][y];
		}

	// update blocks type
	for (int x = 1; x <= map_width; ++x)
		for (int y = 1; y <= map_height; ++y) {
			// skip solid blocks
			if (blocks[x-1][y-1] > Water)  continue;
			
			if (mass[x][y] > min_mass)
				blocks[x-1][y-1] = Water;
			else
				blocks[x-1][y-1] = Air;
		}

	// remove any water that has left map
	for (int x = 0; x < map_width+2; ++x) {
		mass[x][0] = 0;
		mass[x][map_height+1] = 0;
	}

	for (int y = 0; y < map_height; ++y) {
		mass[0][y] = 0;
		mass[map_width+1][y] = 0;
	}
}


void DrawSkyBackground()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	glTranslatef(0, 0, DrawLayer0);
	glScalef(map_width, map_height, 1);
	glBindTexture(GL_TEXTURE_2D, tex_sky);
	glCallList(list_block);
	glPopMatrix();
	glPopAttrib();
}


void DrawDarkBlocks()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.5);

	for (int x = 0; x < map_width; ++x)
		for (int y = 0; y < map_height; ++y)
		{
			if (dark_blocks[x][y] != Air) {
				glPushMatrix();
				glTranslatef(x, y, DrawLayer1);
				glColor4f(0.6, 0.6, 0.6, 1.0);
				glBindTexture(GL_TEXTURE_2D, tex_blocks[dark_blocks[x][y]]);
				glCallList(list_block);
				glPopMatrix();
			}
		}
	glPopAttrib();
}


void DrawWaterBlocks()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	for (int x = 0; x < map_width; ++x)
		for (int y = 0; y < map_height; ++y)
		{
			glBindTexture(GL_TEXTURE_2D, tex_blocks[Water]);
			glColor4f(1, 1, 1, 0.65*mass[x+1][y+1]);

			float h = mass[x+1][y+2] > min_flow? 1.0 : mass[x+1][y+1];
			if (blocks[x][y] == Water) {
				glPushMatrix();
				glTranslatef(x, y, DrawLayer2);
				glScalef(1, h, 1);
				glCallList(list_block);
				glPopMatrix();
			}
		}
	glPopAttrib();
}


void DrawSolidBlocks()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.5);

	if (dark_blocks_drawing == 1) {
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_ALPHA_TEST);
		glBlendFunc(GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		glColor4f(1, 1, 0, 0.35);
	}
	
	for (int i = 0; i < quads.size(); ++i) {
		if (quads[i].type == Water)
			continue;

		int x = quads[i].x, y = quads[i].y;
		int w = quads[i].w, h = quads[i].h;
		
		glPushMatrix();
		glTranslatef(x, y, DrawLayer2);
		glBindTexture(GL_TEXTURE_2D, tex_blocks[blocks[x][y]]);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0); glVertex3f(0, 0, 0);
		glTexCoord2f(w, 0); glVertex3f(w, 0, 0);
		glTexCoord2f(w, h); glVertex3f(w, h, 0);
		glTexCoord2f(0, h); glVertex3f(0, h, 0);
		glEnd();
		glPopMatrix();
	}

	glPopAttrib();
}


void DrawBrushMenu()
{
	float ori_x = (map_width-menu_width) / 2.0f;
	float ori_y = (map_height-menu_height) / 2.0f;

	// draw base
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glBlendFunc(GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glPushMatrix();
	glTranslatef(ori_x-1.5, ori_y-1.5, DrawLayer8);
	glScalef(menu_width+3, menu_height+3, 1);
	glColor4f(0, 0, 1, 0.25);
	glCallList(list_block);
	glPopMatrix();

	glColor4f(1, 1, 1, 1);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	
	// draw the brushes
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.5);
	
	for (int i = 0; i < BlockCount/2; ++i)
	{
		int base = (BlockCount&1)? i+1 : i;
		int shift = i * block_icon_size;
		glPushMatrix();
		glTranslatef(ori_x+shift, ori_y+block_icon_size, DrawLayer7);    //2nd raw
		glScaled(3, 3, 1);
		glBindTexture(GL_TEXTURE_2D, tex_blocks[base+BlockCount/2]);
		glCallList(list_block);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(ori_x+shift, ori_y, DrawLayer8);    //1st raw
		glScaled(3, 3, 1);
		glBindTexture(GL_TEXTURE_2D, tex_blocks[base]);
		glCallList(list_block);
		glPopMatrix();
	}
	glPopAttrib();
}


void DrawSelectTip()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glBlendFunc(GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glPushMatrix();
	glTranslatef(select_xy.x, select_xy.y, DrawLayer9);
	glColor4f(1, 0, 1, 0.5);
	glCallList(list_block);
	glPopMatrix();
	glPopAttrib();
}


void DrawQuadFrames()
{
	// draw greedy meshing result
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushMatrix();
	glTranslatef(0, 0, DrawLayer7);
	glColor4f(1, 0, 0, 1.0);
	glLineWidth(2);

	// dark quads
	for (int i = 0; i < dark_quads.size(); ++i) {
		int x = dark_quads[i].x, y = dark_quads[i].y;
		int w = dark_quads[i].w, h = dark_quads[i].h;
		
		glBegin(GL_LINE_LOOP);
		glVertex3f(x, y, 0); glVertex3f(x+w, y, 0);
		glVertex3f(x+w, y+h, 0); glVertex3f(x, y+h, 0);
		glEnd();
	}
	
	// normal quads
	for (int i = 0; i < quads.size(); ++i) {
		if (quads[i].type == Water)
			continue;

		int x = quads[i].x, y = quads[i].y;
		int w = quads[i].w, h = quads[i].h;
		
		glBegin(GL_LINE_LOOP);
		glVertex3f(x, y, 0); glVertex3f(x+w, y, 0);
		glVertex3f(x+w, y+h, 0); glVertex3f(x, y+h, 0);
		glEnd();
	}
	glPopMatrix();
	glPopAttrib();
}


void OpenGLRendering(sf::RenderWindow* window)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	DrawSkyBackground();
	DrawDarkBlocks();
	DrawWaterBlocks();
	DrawSolidBlocks();

	if (quad_frames == 1)
		DrawQuadFrames();

	if (brush_picking == 1)
		DrawBrushMenu();
	else 
		DrawSelectTip();
}


void ShowTextOverlay(sf::RenderWindow* window, int fps)
{
	char str[32];
	sprintf_s(str, "[FPS: %d]", fps);
	text_fps.setString(str);
	sprintf_s(str, "[Brush: %s]", block_filename[brush]);
	text_selected.setString(str);

	window->pushGLStates();
	window->draw(text_fps);
	window->draw(text_selected);
	window->popGLStates();
}


int main(int argc, char** argv)
{
	sf::ContextSettings context;
    context.depthBits = 24;

	sf::RenderWindow window = sf::RenderWindow(sf::VideoMode(window_width, window_height), 
		"Fluid Simulation 2D", sf::Style::Titlebar | sf::Style::Close, context);
	window.setVerticalSyncEnabled(true);

	// loading textures or something else...
	window.setActive();    // for opengl calls
	InitDemo();

	// start the game loop
	sf::Clock clock;
	while (window.isOpen())
	{
		// get elapsed time
		sf::Time fps_counter = clock.restart();
		// handle events
		sf::Event event;
		while (window.pollEvent(event))
		{
			// window closed
			if (event.type == sf::Event::Closed)
				window.close();
			if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::Tab)
					brush_picking ^= 1;
				if (event.key.code == sf::Keyboard::Space)
					dark_blocks_drawing ^= 1;
				if (event.key.code == sf::Keyboard::F)
					quad_frames ^= 1;
				if (event.key.code == sf::Keyboard::C) {
					// clear the arrays
					for (int x = 0; x < map_width; ++x)
						for (int y = 0; y < map_height; ++y) {
							blocks[x][y] = Air;
							dark_blocks[x][y] = Air;
						}
					for (int x = 0; x < map_width+2; ++x)
						for (int y = 0; y < map_height+2; ++y) {
							mass[x][y] = 0;
							new_mass[x][y] = 0;
						}
					// meshing all
					GreedyMeshing(quads, checked, blocks, map_width, map_height);
					GreedyMeshing(dark_quads, checked, dark_blocks, map_width, map_height);
				}	
			}
		}

		// update the block states
		UpdateBlockStates(&window);
		// simulate fliud
		SimulateFluid();
		// draw opengl rendering here
		OpenGLRendering(&window);
		// show frame rate
		ShowTextOverlay(&window, static_cast<int>(1.0f/fps_counter.asSeconds()));
		// end of the current frame
		window.display();
	}

	return 0;
}