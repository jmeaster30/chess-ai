# chess-ai
This is going to be a CLI Chess game that should work in any unix terminal (I have only tested it on Ubuntu).

Currently, there is no computer player in this program but I do plan on implementing a computer player in the future.

To run the program you need to first "make" it and then run it like in the following commands:
```
make
./chess
--or--
make
./chess <Player 1 Name> <Player 2 Name>
```

If you don't give the program any parameters you will get prompted for the names of each player. Player 1 will always be the white player and Player 2 will always be the black player.

During the game you have a couple commands:
```
move [src] [dest] --moves one of your pieces from src to dest
help [loc]        --gives you a list of possible moves the piece at that location can take
quit              --exits the game
```
