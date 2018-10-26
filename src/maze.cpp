
#include <iostream>
#include "wikipedia_maze.h"
#include "simple_maze.h"

void    TestWikipediaMaze(int argc, char **argv)
{
    //    Node *start, *last;

    size_t  width{0};
    size_t  height{0};

    //Check argument count
    if ( argc < 3 )
    {
        fprintf( stderr, "%s: please specify maze dimensions!\n", argv[0] );
        exit( 1 );
    }

    //Read maze dimensions from command line arguments
    if ( sscanf( argv[1], "%d", &width ) + sscanf( argv[2], "%d", &height ) < 2 )
    {
        fprintf( stderr, "%s: invalid maze size value!\n", argv[0] );
        exit( 1 );
    }

    //Allow only odd dimensions
    if ( !( width % 2 ) || !( height % 2 ) )
    {
        fprintf( stderr, "%s: dimensions must be odd!\n", argv[0] );
        exit( 1 );
    }

    //Do not allow negative dimensions
    if ( width <= 0 || height <= 0 )
    {
        fprintf( stderr, "%s: dimensions must be greater than 0!\n", argv[0] );
        exit( 1 );
    }

    WikiMaze(width, height).Run();

    //    //Seed random generator
    //    srand( time( NULL ) );

    //    //Initialize maze
    //    if ( init( ) )
    //    {
    //        fprintf( stderr, "%s: out of memory!\n", argv[0] );
    //        exit( 1 );
    //    }

    //    //Setup start node
    //    start = nodes + 1 + width;
    //    start->parent = start;
    //    last = start;

    //    //Connect nodes until start node is reached and can't be left
    //    while ( ( last = link( last ) ) != start );
    //    draw( );
}


void    TestSimpleMaze()
{
    srand(time(nullptr));
    SimpleMaze maze;

    maze.Setup(20, 20);
//    maze.Draw("../maze_raw.svg");
//    maze.Dump("../maze_raw.txt");

    maze.Iterate();

    maze.Draw("../maze.svg");
//    maze.Dump("../maze.txt");
    return;
}

int main(int argc, char **argv)
{
//    TestWikipediaMaze(argc, argv);
    TestSimpleMaze();
    return 0;
}
