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
void on_generate_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget **widgets = (GtkWidget **)data;
    GtkWidget *entry = widgets[0];
    GtkWidget *length_entry = widgets[1];
    
    const char *length_str = gtk_entry_get_text(GTK_ENTRY(length_entry));
    int length = atoi(length_str);
    if (length <= 0) length = 12;  // Default length
    
    char *password = (char *)malloc((length + 1) * sizeof(char));
    generate_password(password, length);
    gtk_entry_set_text(GTK_ENTRY(entry), password);
    free(password);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    srand(time(NULL)); // Seed the random number generator
    
    GtkWidget *window;
    GtkWidget *button;
    GtkWidget *entry;
    GtkWidget *length_entry;
    GtkWidget *box;
    
    // Create a new window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Password Generator");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_widget_set_size_request(window, 300, 200);
    
    // Create a vertical box
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), box);
    
    // Create an entry widget for the generated password
    entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(box), entry, FALSE, FALSE, 5);
    
    // Create an entry widget for the length input
    length_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(length_entry), "Enter length (default 12)");
    gtk_box_pack_start(GTK_BOX(box), length_entry, FALSE, FALSE, 5);
    
    // Create a button widget
    button = gtk_button_new_with_label("Generate Password");
    GtkWidget *widgets[] = {entry, length_entry};
    g_signal_connect(button, "clicked", G_CALLBACK(on_generate_clicked), widgets);
    gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 5);
    
    // Show all widgets
    gtk_widget_show_all(window);
    
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    gtk_main();

    g_free(widgets);
    
    return 0;
}

