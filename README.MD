# GUI Password Generator

This simple app shows you very useful example of gtk3 app.

## How to install?

At first you have to install gtk3:

> sudo apt-get install libgtk-3-dev

Then compile this app with:

> gcc -o main main.c `pkg-config --cflags --libs gtk+-3.0`

And finally run her with:

> ./main

## Some Tips

1) In your password try to replace '0' with 'O' (letter) and vice versa (e.g: LEM0N LEMON)
2) Try to achieve length of at least 12 symbols.
3) Try not to have any real words in your password, NEVER
