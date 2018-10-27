#ifndef SIMPLE_MAZE_H
#define SIMPLE_MAZE_H


#include <fstream>
#include <string>
#include <vector>
#include <cmath>

#include <pugixml.hpp>

class SimpleMazeBase
{
protected:
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

        void    DrawLine(pugi::xml_node &node, Point from, Point to, std::string color, size_t door_index)
        {
            from.x = std::round(from.x);
            from.y = std::round(from.y);
            to.x = std::round(to.x);
            to.y = std::round(to.y);

            auto line = node.append_child("line");
            line.append_attribute("room").set_value(this->index);
            line.append_attribute("door_dir").set_value(door_index);
            line.append_attribute("x1").set_value(from.x);
            line.append_attribute("y1").set_value(from.y);
            line.append_attribute("x2").set_value(to.x);
            line.append_attribute("y2").set_value(to.y);
            line.append_attribute("stroke-width").set_value(4);
            line.append_attribute("stroke").set_value(color.c_str());
            line.append_attribute("opacity").set_value(1);
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

    size_t  width{0};
    size_t  height{0};

    std::vector<Door*>  doors;
    std::vector<Room>  rooms;

public:
    SimpleMazeBase() {}
    ~SimpleMazeBase()
    {
        for (auto &door : doors)
        {
            delete door;
            door = nullptr;
        }
    }

    void    Iterate()
    {
        auto    &room = rooms[Index(10,10)];
        room.Visit();
        Iterate(&room);
    }

};

class SimpleMaze : public SimpleMazeBase
{
public:
    SimpleMaze() : SimpleMazeBase() {}

    void    Setup(size_t width, size_t height)
    {
        this->width = width;
        this->height = height;

        rooms.resize(width * height, Room(4));

        // Top row:
        for (size_t w = 0; w<width - 1; ++w)
        {
            // Connect to the east:
            auto door_east = new Door();
            doors.push_back(door_east);

            door_east->Connect(&rooms[Index(w, 0)], &rooms[Index(w + 1, 0)]);
            rooms[Index(w, 0)].doors[1] = door_east;
            rooms[Index(w + 1, 0)].doors[3] = door_east;
        }

        for (size_t h = 1; h<height; ++h)
        {
            for (size_t w = 0; w<width - 1; ++w)
            {
                // Connect to the east:
                auto door_east = new Door();
                doors.push_back(door_east);

                door_east->Connect(&rooms[Index(w, h)], &rooms[Index(w + 1, h)]);
                rooms[Index(w, h)].doors[1] = door_east;
                rooms[Index(w + 1, h)].doors[3] = door_east;

                // Connect to the north:
                auto door_north = new Door();
                doors.push_back(door_north);

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
            auto door_north = new Door();
            doors.push_back(door_north);

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
            doors[i]->SetIndex(i);
        }
    }

    void    Draw(std::string filename)
    {
        for (auto &door : doors)
        {
            door->ResetDrawn();
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
