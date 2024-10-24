#include <gtk/gtk.h>
#include <stdlib.h>
#include <time.h>

// Function to generate a random password
void generate_password(char *password, int length) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (int i = 0; i < length; i++) {
        int key = rand() % (int)(sizeof(charset) - 1);
        password[i] = charset[key];
    }
    password[length] = '\0';
}

// Callback function for the button
void on_generate_clicked(GtkWidget *widget, gpointer entry) {
    char password[13]; // Password length: 12 + 1 for the null terminator
    generate_password(password, 12);
    gtk_entry_set_text(GTK_ENTRY(entry), password);  // Fix the argument issue
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    srand(time(NULL)); // Seed the random number generator
    
    GtkWidget *window;
    GtkWidget *button;
    GtkWidget *entry;
    GtkWidget *box;
    
    // Create a new window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Password Generator");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_widget_set_size_request(window, 300, 200);
    
    // Create a vertical box
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), box);
    
    // Create an entry widget
    entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(box), entry, FALSE, FALSE, 5);
    
    // Create a button widget
    button = gtk_button_new_with_label("Generate Password");
    g_signal_connect(button, "clicked", G_CALLBACK(on_generate_clicked), entry);
    gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 5);
    
    // Show all widgets
    gtk_widget_show_all(window);
    
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    gtk_main();
    
    return 0;
}

