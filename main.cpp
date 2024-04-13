#include <stdio.h>
#include <iostream>
#include "CommonFunc.h"
#include "BaseObject.h"
#include "gamemap.h"
#include "MainObject.h"
#include "ImpTimer.h"
#include "ThreatObject.h"
#include "PlayHealth.h"
#include "TextObject.h"

BaseObject g_background;
TTF_Font *font_time = NULL;
TTF_Font *font_money = NULL;
TTF_Font *gFont1 = NULL;
TTF_Font *gFont2 = NULL;
TTF_Font *gFont3 = NULL;
TTF_Font *gFont4 = NULL;


void Restart(Map& map_data, int& num_die, int& heart_count, MainObject& p_player);


bool InitData()
{
    bool success = true;
    int ret = SDL_Init(SDL_INIT_VIDEO);
    if (ret < 0)
        return false;

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    g_window = SDL_CreateWindow("Game 2d",
                                SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED,
                                SCREEN_WIDTH, SCREEN_HEIGHT,
                                SDL_WINDOW_SHOWN);
    if (g_window == NULL)
    {
        success = false;
    }

    else
    {
        g_screen = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED);
        if (g_screen == NULL)
            success = false;
        else
        {
            SDL_SetRenderDrawColor(g_screen, RENDER_DRAW_COLOR, RENDER_DRAW_COLOR, RENDER_DRAW_COLOR, RENDER_DRAW_COLOR);
            int imgFlags = IMG_INIT_PNG;
            if (!(IMG_Init(imgFlags) && imgFlags))
                success = false;
        }

        if (TTF_Init() == -1)
        {
            success = false;
            std::cout << "khong the mo tep";
        }

        font_time = TTF_OpenFont("font/1.ttf", 35);
        font_money = TTF_OpenFont("font/1.ttf", SIZE_FONT_HEART);

        if (font_time == NULL)
        {
            success = false;
            std::cout << "khong the mo tep ";
        }
    }

    return success;
}

bool LoadBackground()
{
    bool ret = g_background.LoadImg("img/background.jpg", g_screen);
    if (ret == false)
        return false;

    return true;
}

void close()
{
    g_background.Free();

    SDL_DestroyRenderer(g_screen);
    g_screen = NULL;

    SDL_DestroyWindow(g_window);
    g_window = NULL;

    IMG_Quit();
    SDL_Quit();
}

std::vector<ThreatsObject *> MakeThreats()
{
    std::vector<ThreatsObject *> list_threats;

    ThreatsObject *dynamic_threats = new ThreatsObject[NUM_THREATS_LIST];

    for (int i = 0; i < NUM_THREATS_LIST; i++)
    {
        ThreatsObject *p_threat = (dynamic_threats + i);

        if (p_threat != NULL)
        {
            p_threat->LoadImg("img/threat_2_left.png", g_screen);
            p_threat->set_clips();
            p_threat->set_type_move(ThreatsObject::MOVE_INSPACE_THREAT);
            p_threat->set_x_pos(500 + i * 1000);
            p_threat->set_y_pos(200);

            int pos1 = p_threat->get_x_pos() - 200;
            int pos2 = p_threat->get_x_pos() + 200;
            p_threat->SetAnimationPos(pos1, pos2);
            p_threat->set_input_left(1);

            list_threats.push_back(p_threat);
        }
    }

    ThreatsObject *ThreatFly = new ThreatsObject[NUM_THREATS_LIST];

    for (int i = 0; i < NUM_THREATS_LIST; i++)
    {
        ThreatsObject *p_threat = (ThreatFly + i);
        if (p_threat != NULL)
        {
            p_threat->LoadImg("img/threat_3_left.png", g_screen); // loading threats
            p_threat->set_clips();
            p_threat->set_x_pos(290 + i * 1000);
            p_threat->set_y_pos(250);
            p_threat->set_type_move(ThreatsObject::THREATS_FLY_STATIC);

            list_threats.push_back(p_threat);
        }
    }

    return list_threats;
}

/////

void renderText(const std::string &text, int x, int y, TTF_Font *font)
{
    SDL_Color textColor = {255, 255, 255}; // White color
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, text.c_str(), textColor);
    if (textSurface == nullptr)
    {
        std::cerr << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(g_screen, textSurface);
    if (texture == nullptr)
    {
        std::cerr << "Unable to create texture from rendered text! SDL Error: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(textSurface);
        return;
    }

    SDL_Rect dstRect = {x, y, textSurface->w, textSurface->h};
    SDL_RenderCopy(g_screen, texture, nullptr, &dstRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(texture);
}

///

int main(int argc, char *argv[])
{
    ImpTimer fps_timer;

    if (InitData() == false)
        return -1;

    if (LoadBackground() == false)
        return -1;

    GameMap game_map;
    game_map.LoadMap("map/map01.txt");
    game_map.LoadTiles(g_screen);

    MainObject p_player;
    p_player.LoadImg("img/player_right1.png", g_screen);
    p_player.set_clips();

    PlayerPower player_power;
    player_power.Init(g_screen);

    PlayerMoney player_heart;
    player_heart.Init(g_screen);
    player_heart.SetPos(SCREEN_WIDTH * 0.5 - 12, 42);

    std::vector<ThreatsObject *> threats_list = MakeThreats();

    int num_die = 0;

    // Time Text
    TextObject time_game;
    time_game.SetColor(TextObject::WHITE_TEXT);

    TextObject heart_game;
    heart_game.SetColor(TextObject::RED_TEXT);

    bool is_quit = false;

    bool start = false;
    while (start == false)
    {
        SDL_Surface *g_img_menu;
        gFont1 = TTF_OpenFont("font/2.ttf", 30);
        gFont2 = TTF_OpenFont("font/2.ttf",30);
        g_img_menu = IMG_Load("menu/menu.png");

        SDL_Texture *menu = SDL_CreateTextureFromSurface(g_screen, g_img_menu);
        SDL_Rect menuRect = {0, 0, g_img_menu->w, g_img_menu->h};

        SDL_RenderCopy(g_screen, menu, NULL, &menuRect);

        renderText("SPACE TO START!", SCREEN_WIDTH-300 , 420, gFont1);
        renderText("ESC TO EXIT!", 30, 420, gFont2);

        SDL_RenderPresent(g_screen);
        SDL_FreeSurface(g_img_menu);
        SDL_DestroyTexture(menu);



        SDL_Event eve;

        while (SDL_PollEvent(&eve))
        {
            if (eve.type == SDL_KEYDOWN && eve.key.keysym.sym == SDLK_SPACE)
            {
                start = true;
                break;
            }
            if(eve.type == SDL_KEYDOWN && eve.key.keysym.sym == SDLK_ESCAPE)
            {
                close();
                return 0;
            }
        }

    }


    while (!is_quit)
    {
        int heart_count = p_player.GetMoneyCount();


        fps_timer.start();
        while (SDL_PollEvent(&g_event) != 0)
        {
            if (g_event.type == SDL_QUIT)
            {
                is_quit = true;
            }

            p_player.HandelInputAction(g_event, g_screen);
        }

        SDL_SetRenderDrawColor(g_screen, RENDER_DRAW_COLOR, RENDER_DRAW_COLOR, RENDER_DRAW_COLOR, RENDER_DRAW_COLOR);
        SDL_RenderClear(g_screen);

        g_background.Render(g_screen, NULL);

        Map map_data = game_map.getMap();

        p_player.HanleBullet(g_screen);
        p_player.SetMapXY(map_data.start_x_, map_data.start_y_);
        p_player.DoPlayer(map_data);
        p_player.Show(g_screen);

        game_map.SetMap(map_data);
        game_map.DrawMap(g_screen);

        player_power.Show(g_screen);
        player_heart.Show(g_screen);

        bool is_minusLinve = p_player.GetIsMinusLive();

        bool bCol2 = false;
        for (int i = 0; i < threats_list.size(); i++)
        {
            ThreatsObject *p_threat = threats_list.at(i);
            if (p_threat != NULL)
            {
                p_threat->SetMapXY(map_data.start_x_, map_data.start_y_);
                p_threat->ImpMoveType(g_screen);
                p_threat->DoPlayer(map_data);
                p_threat->Show(g_screen);

                SDL_Rect rect_player = p_player.GetRectFrame();
                SDL_Rect rect_threat = p_threat->GetRectFrame();
                bCol2 = SDLCommonFunc::CheckCollision(rect_player, rect_threat);
                if (bCol2 == true)
                {
                    p_threat->Free();
                    break;
                }
            }
        }

        if (bCol2 || is_minusLinve == true)
        {
            num_die++;
            if (is_minusLinve == true)
            {
                p_player.RetsetMinusLive();
            }

            if (num_die <= 3)
            {
                p_player.SetRect(0, 0);
                p_player.set_comeback_time(3);
                SDL_Delay(1000);
                player_power.Decrease();
                player_power.Render(g_screen);
                continue;
            }
            else
            {
                bool quit_game_over=false;

                while (quit_game_over == false)
                {
                    gFont3 = TTF_OpenFont("font/1.ttf", 120);
                    renderText("POINT: ", SCREEN_WIDTH / 2 - 300, 220, gFont3);
                    renderText(std::to_string(heart_count).c_str(), SCREEN_WIDTH / 2 + 20, 220, gFont3);


                    SDL_Delay(2000);
                    gFont4 = TTF_OpenFont("font/2.ttf", 100);
                    renderText("SPACE TO REPLAY!", SCREEN_WIDTH / 2 - 420, 380, gFont3);
                    SDL_RenderPresent(g_screen);
                    SDL_Event even;

                    while (SDL_PollEvent(&even))
                    {
                        if (even.type == SDL_KEYDOWN && even.key.keysym.sym == SDLK_SPACE)
                        {
//////


                            std::cout << "Restart";
                            Restart(map_data, num_die, heart_count, p_player);


///////////////////
                            quit_game_over=true;
                            break;
                        }
                        if (even.type == SDL_KEYDOWN && even.key.keysym.sym == SDLK_ESCAPE)
                        {
                            quit_game_over=true;
                            is_quit=true;
                        }
                    }
                }

/////////////


            }
            for(int i=0;i<8000;i++)
            {
                bCol2=false;
            }

 
        }

        std::vector<BulletObject *> bullet_arr = p_player.get_bullet_list();
        for (int r = 0; r < bullet_arr.size(); r++)
        {
            BulletObject *p_bullet = bullet_arr.at(r);
            {
                if (p_bullet != NULL)
                {
                    for (int t = 0; t < threats_list.size(); t++)
                    {
                        ThreatsObject *obj_threat = threats_list.at(t);
                        if (obj_threat != NULL)
                        {
                            SDL_Rect tRect;
                            tRect.x = obj_threat->GetRect().x;
                            tRect.y = obj_threat->GetRect().y;
                            tRect.w = obj_threat->get_width_frame();
                            tRect.h = obj_threat->get_height_frame();

                            SDL_Rect bRect = p_bullet->GetRect();

                            bool bCol = SDLCommonFunc::CheckCollision(bRect, tRect);

                            if (bCol)
                            {
                                p_player.RemoveBullet(r);
                                obj_threat->Free();
                                threats_list.erase(threats_list.begin() + t);
                            }
                        }
                    }
                }
            }
        }

        // Show game time
        std::string str_time = "Days: ";
        Uint32 time_val = SDL_GetTicks() / 1000;
        Uint32 val_time = 0 + time_val;
        if (val_time >= 5000)
        {
            if (MessageBoxW(NULL, L"T-kun lost her!", L"Info", MB_OK | MB_ICONSTOP) == IDOK)
            {
                is_quit = true;
                break;
            }
        }
        else
        {
            std::string str_val = std::to_string(val_time);
            str_time += str_val;

            time_game.SetText(str_time);
            time_game.LoadFromRenderText(font_time, g_screen);
            time_game.RenderText(g_screen, SCREEN_WIDTH - 200, 15);
        }

        std::string heart_str = std::to_string(heart_count);

        heart_game.SetText(heart_str);
        heart_game.LoadFromRenderText(font_money, g_screen);
        heart_game.RenderText(g_screen, SCREEN_WIDTH * 0.5, 5);

        SDL_RenderPresent(g_screen);

        int real_imp_time = fps_timer.get_ticks();
        int time_one_frame = 1000 / FRAME_PER_SECOND;

        if (real_imp_time < time_one_frame)
        {
            int delay_time = time_one_frame - real_imp_time;
            if (delay_time >= 0)
                SDL_Delay(delay_time);
        }
    }

    close();
    return 0;
}


void Restart(Map& map_data, int& num_die, int& heart_count, MainObject& p_player)
{
    // Thiết lập lại vị trí ban đầu của bản đồ
    map_data.start_x_ = 0;
    map_data.start_y_ = 0;
    
    // Thiết lập lại số lần chết và điểm
    num_die = 0;
    heart_count = 0;

    // Thiết lập lại vị trí và trạng thái của người chơi

    p_player.SetRect(0, 0); // Thiết lập lại vị trí
    p_player.set_comeback_time(3); // Thiết lập thời gian quay lại mặc định (nếu có)
}
