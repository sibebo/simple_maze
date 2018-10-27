#ifndef SIMPLE_MAZE_H
#define SIMPLE_MAZE_H


#include <iostream>
#include <fstream>
//#include <stdio.h>
//#include <stdlib.h>
//#include <time.h>
//#include <string.h>
#include <string>
#include <vector>
#include <cmath>

#include <pugixml.hpp>

class SimpleMaze
{
    class Point
    {
    public:
        double  x{0};
        double  y{0};

        Point() {}
        Point(double x, double y) : x(x), y(y) {}
        Point(size_t x, size_t y) : x(static_cast<double>(x)), y(static_cast<double>(y)) {}

        friend  Point   operator+(const Point &a, const Point &b) {return Point(a.x + b.x, a.y + b.y);}
        friend  Point   operator-(const Point &a, const Point &b) {return Point(a.x - b.x, a.y - b.y);}
        friend  Point   operator*(const Point &a, double factor) {return Point(a.x * factor, a.y * factor);}
        friend  Point   operator*(double factor, const Point &a) {return Point(a.x * factor, a.y * factor);}
        friend  Point   operator/(const Point &a, double factor) {return Point(a.x / factor, a.y / factor);}
    };

    class Room;

    class Door
    {
        Room    *behind_the_door[2] = {nullptr, nullptr};
        bool    is_open{false};
        bool    is_drawn{false};
        size_t  index{0};

    public:
        Door() {}

        void    SetIndex(size_t index) {this->index = index;}

        void    Connect(Room *a, Room *b) {behind_the_door[0] = a; behind_the_door[1] = b;}

        bool    IsLocked() const {return (behind_the_door[0] == nullptr) || (behind_the_door[1] == nullptr);}
        bool    IsOpen() const {return is_open;}
        void    Open() {is_open = true;}
        void    Close() {is_open = false;}
        bool    IsDrawn() const {return is_drawn;}
        void    ResetDrawn() {is_drawn = false;}
        void    SetDrawn() {is_drawn = true;}

        Room*   BehindTheDoor(const Room *me) const
        {
            if (IsLocked()) return nullptr;

            if (me == behind_the_door[0])
            {
                return behind_the_door[1];
            }
            else if (me == behind_the_door[1])
            {
                return behind_the_door[0];
            }
            else
            {
                return nullptr;
            }
        }
    };

    class Room
    {
        std::vector<Door*>  DoorsToUnvisitedNeighbours() const
        {
            std::vector<Door*>  doors_to_unvisited_rooms;

            for (Door *door : doors)
            {
                if (door == nullptr) continue;  // room is invalid.
                if (door->IsLocked()) continue; // no room behind the door.

                if (!door->BehindTheDoor(this)->IsVisited())
                {
                    doors_to_unvisited_rooms.push_back(door);
                }
            }

            return doors_to_unvisited_rooms;
        }

    public:
        size_t      index{0};
        std::vector<Door*>  doors;
        bool    is_visited{false};

        Room(size_t door_count) : doors(door_count, nullptr) {}

        bool    IsVisited() const {return is_visited;}
        void    Visit() {is_visited = true;}

        void    SetIndex(size_t index) {this->index = index;}

        Room*   EnterUnvisitedNeighbour()
        {
            auto    doors_to_unvisited_neighbours = DoorsToUnvisitedNeighbours();

            if (doors_to_unvisited_neighbours.empty()) return nullptr;

            auto    door_index = (rand() % doors_to_unvisited_neighbours.size());

            auto    door = doors_to_unvisited_neighbours[door_index];
            door->Open();

            auto    room = door->BehindTheDoor(this);
            room->Visit();

            return room;
        }

        void    DrawRoom(pugi::xml_node &node, const Point &where, double scale)
        {
            double  sweep_angle = 8.0 * std::atan(1.0) / doors.size() / 2.0;
            double  r = 1.0 / std::cos(sweep_angle) / 2.0;

            double  direction_step = 2.0 * sweep_angle;
            double  direction{-direction_step};

            size_t  door_index{0};
            for (auto &door : doors)
            {
                auto p1 = r * Point(cos(direction - sweep_angle), sin(direction - sweep_angle));
                auto p2 = r * Point(cos(direction + sweep_angle), sin(direction + sweep_angle));

                auto pa = scale * (where + Point(0.5, 0.5) + p1);
                auto pb = scale * (where + Point(0.5, 0.5) + p2);

                if (door != nullptr)
                {
                    if (!door->IsOpen() && !door->IsDrawn())
                    {
                            DrawLine(node, pa, pb, "green", door_index);
                            door->SetDrawn();
                    }
                }
                else
                {
                    DrawLine(node, pa, pb, "red", door_index);
                }

                direction += direction_step;
                ++door_index;
            }
        }



        void    DrawLine(pugi::xml_node &node, Point from, Point to, std::string color, size_t door_index)
        {
            from.x = std::round(from.x);
            from.y = std::round(from.y);
            to.x = std::round(to.x);
            to.y = std::round(to.y);

            auto line = node.append_child("line");
            line.append_attribute("room").set_value(this->index);
//            line.append_attribute("door").set_value(this->doors[door_index]->index);
            line.append_attribute("door_dir").set_value(door_index);
            line.append_attribute("x1").set_value(from.x);
            line.append_attribute("y1").set_value(from.y);
            line.append_attribute("x2").set_value(to.x);
            line.append_attribute("y2").set_value(to.y);
            line.append_attribute("stroke-width").set_value(4);
            line.append_attribute("stroke").set_value(color.c_str());
            line.append_attribute("opacity").set_value(1);
        }
    };

    void    Iterate(Room *room)
    {
        if (room == nullptr)
        {
            return;
        }

        Room    *next_room;
        while ((next_room = room->EnterUnvisitedNeighbour()) != nullptr)
        {
            Iterate(next_room);
        }
    }

    size_t  Index(size_t w, size_t h)
    {
        return h * width + w;
    }

    void    Draw(pugi::xml_node &svg, size_t x1, size_t y1, size_t x2, size_t y2, std::string color, size_t room_index, size_t door_index)
    {
        //<line x1="0" y1="0" x2="200" y2="200" style="stroke:rgb(255,0,0);stroke-width:2" />
        auto    line = svg.append_child("line");
        line.append_attribute("room").set_value(room_index);
//        line.append_attribute("door").set_value(this->doors[door_index]->index);
        line.append_attribute("door_dir").set_value(door_index);
        line.append_attribute("x1").set_value(x1);
        line.append_attribute("y1").set_value(y1);
        line.append_attribute("x2").set_value(x2);
        line.append_attribute("y2").set_value(y2);
        //line.append_attribute("style").set_value("stroke:black;stroke-width:1;");
        line.append_attribute("stroke-width").set_value(4);
        line.append_attribute("stroke").set_value(color.c_str());
        line.append_attribute("opacity").set_value(1);
    }

    void    Draw(pugi::xml_node &svg, size_t w, size_t h, Door *door, size_t direction, size_t room_index)
    {
        size_t  x1{0};
        size_t  y1{0};
        size_t  x2{0};
        size_t  y2{0};

        size_t  scale{30};

        switch(direction)
        {
        case 0:
            x1 = (w + 0) * scale;
            y1 = (h + 0) * scale;
            x2 = (w + 1) * scale;
            y2 = (h + 0) * scale;
            break;
        case 1:
            x1 = (w + 1) * scale;
            y1 = (h + 0) * scale;
            x2 = (w + 1) * scale;
            y2 = (h + 1) * scale;
            break;
        case 2:
            x1 = (w + 1) * scale;
            y1 = (h + 1) * scale;
            x2 = (w + 0) * scale;
            y2 = (h + 1) * scale;
            break;
        case 3:
            x1 = (w + 0) * scale;
            y1 = (h + 1) * scale;
            x2 = (w + 0) * scale;
            y2 = (h + 0) * scale;
            break;
        }

        if (door)
        {
            if (!door->IsOpen() && !door->IsDrawn())
            {
                Draw(svg, x1, y1, x2, y2, "black", room_index, direction);
                door->SetDrawn();
            }
        }
        else
        {
            Draw(svg, x1, y1, x2, y2, "red", room_index, direction);
        }
    }

    size_t  width{0};
    size_t  height{0};

    std::vector<Door>  doors;
    std::vector<Room>  rooms;

public:
    SimpleMaze() {}

    void    Setup(size_t width, size_t height)
    {
        this->width = width;
        this->height = height;

        rooms.resize(width * height, Room(4));
        doors.reserve(width * height * 4);

        // Top row:
        for (size_t w = 0; w<width - 1; ++w)
        {
            // Connect to the east:
            doors.emplace_back();
            auto door_east = &doors.back();
            door_east->Connect(&rooms[Index(w, 0)], &rooms[Index(w + 1, 0)]);
            rooms[Index(w, 0)].doors[1] = door_east;
            rooms[Index(w + 1, 0)].doors[3] = door_east;
        }

        for (size_t h = 1; h<height; ++h)
        {
            for (size_t w = 0; w<width - 1; ++w)
            {
                // Connect to the east:
                doors.emplace_back();
                auto door_east = &doors.back();
                door_east->Connect(&rooms[Index(w, h)], &rooms[Index(w + 1, h)]);
                rooms[Index(w, h)].doors[1] = door_east;
                rooms[Index(w + 1, h)].doors[3] = door_east;

                // Connect to the north:
                doors.emplace_back();
                auto door_north = &doors.back();
                door_north->Connect(&rooms[Index(w, h)], &rooms[Index(w, h - 1)]);
                rooms[Index(w, h)].doors[0] = door_north;
                rooms[Index(w, h - 1)].doors[2] = door_north;
            }
        }

        // Northern doors in right hand column:
        for (size_t h = 1; h<height; ++h)
        {
            auto w = width - 1;
            // Connect to the north:
            doors.emplace_back();
            auto door_north = &doors.back();
            door_north->Connect(&rooms[Index(w, h)], &rooms[Index(w, h - 1)]);
            rooms[Index(w, h)].doors[0] = door_north;
            rooms[Index(w, h - 1)].doors[2] = door_north;

        }

        for (size_t i=0; i<rooms.size(); ++i)
        {
            rooms[i].SetIndex(i);
        }
        for (size_t i=0; i<doors.size(); ++i)
        {
            doors[i].SetIndex(i);
        }
    }

    void    Iterate()
    {
        auto    &room = rooms[Index(10,10)];
        room.Visit();
        Iterate(&room);
    }

    void    Draw(std::string filename)
    {
        for (auto &door : doors)
        {
            door.ResetDrawn();
        }

        pugi::xml_document  doc;

        auto    svg = doc.append_child("svg");

        for (size_t h = 0; h<height; ++h)
        {
            for (size_t w = 0; w<width; ++w)
            {
                auto    &r = rooms[Index(w, h)];

                size_t  index{0};
                for (auto &door : r.doors)
                {
                    Draw(svg, w, h, door, index++, r.index);
                }
            }
        }

        doc.save_file(filename.c_str());
    }

    void    DrawNew(std::string filename)
    {
        for (auto &door : doors)
        {
            door.ResetDrawn();
        }

        pugi::xml_document  doc;

        auto    svg = doc.append_child("svg");

        for (size_t h = 0; h<height; ++h)
        {
            for (size_t w = 0; w<width; ++w)
            {
                auto    &r = rooms[Index(w, h)];
                r.DrawRoom(svg, {w,h}, 30);
            }
        }

        doc.save_file(filename.c_str());
    }

    void    Dump(std::string filename)
    {
        std::ofstream   file(filename);

        for (size_t h=0; h<width; ++h)
        {
            for (size_t w=0; w<width; ++w)
            {
                auto &r = rooms[Index(w, h)];
                for (auto d : r.doors)
                {
                    if (d == nullptr)
                    {
                        file << "#";
                    }
                    else if (d->IsLocked())
                    {
                        file << "x";
                    }
                    else if (d->IsOpen())
                    {
                        file << "O";
                    }
                    else
                    {
                        file << "-";
                    }

                }
                file << '\t';
            }
            file << '\n';
        }
    }
};



#endif // SIMPLE_MAZE_H
