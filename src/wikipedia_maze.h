#ifndef WIKIPEDIA_MAZE_H
#define WIKIPEDIA_MAZE_H

//Code by Jacek Wieczorek

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <string>
#include <vector>


struct Node
{
    size_t  x, y; //Node position - little waste of memory, but it allows faster generation
    Node *parent; //Pointer to parent node
    char c; //Character to be displayed
    char dirs; //Directions that still haven't been explored
};


class WikiMaze
{
    //    Node    *nodes{nullptr}; //Nodes array
    size_t     width{0};
    size_t     height{0}; //Maze dimensions
    std::vector<Node>   nodes;
public:
    WikiMaze(size_t width, size_t height) : width(width), height(height), nodes(width * height) {}

    int init( )
    {
        //        int i, j;
        //        Node *n;

        //        //Allocate memory for maze
        //        nodes = calloc( width * height, sizeof( Node ) );
        //        if ( nodes == NULL ) return 1;

        //Setup crucial nodes
        for ( size_t i = 0; i < width; i++ )
        {
            for ( size_t j = 0; j < height; j++ )
            {
                //                n = nodes + i + j * width;
                Node &n = nodes[i + j * width];
                if ( i * j % 2 )
                {
                    n.x = i;
                    n.y = j;
                    n.dirs = 0b1111; //15; //Assume that all directions can be explored (4 youngest bits set)
                    n.c = ' ';
                }
                else n.c = '#'; //Add walls between nodes
            }
        }
        return 0;
    }

    int Run()
    {
        //        Node *start, *last;

        //Seed random generator
        srand( time( nullptr ) );

        //Initialize maze
        if ( init( ) )
        {
            //fprintf( stderr, "%s: out of memory!\n", argv[0] );
            exit( 1 );
        }

        //Setup start node
        Node    *start = &nodes[1 + width];
        start->parent = start;

        Node    *last = start;

        //Connect nodes until start node is reached and can't be left
        while ( (last = link(last)) != start )
        {;}

        draw( );

        return 0;
    }

    Node*   link( Node *n )
    {
        //Connects node to random neighbor (if possible) and returns
        //address of next node that should be visited

        size_t x{0}, y{0};
        //char dir;
        //Node *dest;

        //Nothing can be done if null pointer is given - return
        if ( n == nullptr ) return nullptr;

        //While there are directions still unexplored
        while ( n->dirs )
        {
            //Randomly pick one direction
            char    dir = ( 1 << ( rand() % 4 ) );

            //If it has already been explored - try again
            if ( ~n->dirs & dir ) continue;

            //Mark direction as explored
            n->dirs &= ~dir;

            //Depending on chosen direction
            switch ( dir )
            {
            //Check if it's possible to go right
            case 1:
                if ( n->x + 2 < width )
                {
                    x = n->x + 2;
                    y = n->y;
                }
                else continue;
                break;

                //Check if it's possible to go down
            case 2:
                if ( n->y + 2 < height )
                {
                    x = n->x;
                    y = n->y + 2;
                }
                else continue;
                break;

                //Check if it's possible to go left
            case 4:
                if ( n->x >= 2 )
                {
                    x = n->x - 2;
                    y = n->y;
                }
                else continue;
                break;

                //Check if it's possible to go up
            case 8:
                if ( n->y >= 2 )
                {
                    x = n->x;
                    y = n->y - 2;
                }
                else continue;
                break;
            }

            //Get destination node into pointer (makes things a tiny bit faster)
            //dest = nodes + x + y * width;
            Node *dest = &nodes[x + y * width];

            //Make sure that destination node is not a wall
            if ( dest->c == ' ' )
            {
                //If destination is a linked node already - abort
                if ( dest->parent != nullptr ) continue;

                //Otherwise, adopt node
                dest->parent = n;

                //Remove wall between nodes
                nodes[n->x + (x - n->x) / 2 + (n->y + (y - n->y) / 2) * width].c = ' ';

                //Return address of the child node
                return dest;
            }
        }

        //If nothing more can be done here - return parent's address
        return n->parent;
    }

    void draw( )
    {
        //int i, j;

        //Outputs maze to terminal - nothing special
        for ( size_t i = 0; i < height; i++ )
        {
            for ( size_t j = 0; j < width; j++ )
            {
                printf( "%c", nodes[j + i * width].c );
            }
            printf( "\n" );
        }
    }
};




#endif  // WIKIPEDIA_MAZE_H
