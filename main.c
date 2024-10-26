#include <gtk/gtk.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>


// version = 0.1

// Callback to clear the entry
static gboolean clear_entry(gpointer user_data) {
    GtkWidget **widgets_clear = (GtkWidget **)user_data;
    GtkToggleButton *toggle_button = GTK_TOGGLE_BUTTON(widgets_clear[0]);
    GtkEntry *entry = GTK_ENTRY(widgets_clear[1]);
    
    // Check if the toggle button is active
    if (gtk_toggle_button_get_active(toggle_button)) {
        gtk_entry_set_text(entry, "");
    }

    // Return TRUE to keep the timer running
    return TRUE;
}

static void copy_to_clipboard(GtkWidget *button, gpointer user_data) {
    GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    GtkEntry *entry = GTK_ENTRY(user_data);
    const gchar *text = gtk_entry_get_text(entry);
    gtk_clipboard_set_text(clipboard, text, -1);
    g_print("Text copied to clipboard: %s\n", text);
}

// Function to generate a random password with custom character sets
void generate_password(char *password, int length, int use_lower, int use_upper, int use_digits, int use_special) {
    const char lower[] = "abcdefghijklmnopqrstuvwxyz";
    const char upper[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const char digits[] = "0123456789";
    const char special[] = "!@#$%^&*";
    char charset[128] = "";
    
    if (use_lower) strcat(charset, lower);
    if (use_upper) strcat(charset, upper);
    if (use_digits) strcat(charset, digits);
    if (use_special) strcat(charset, special);
    
    int charset_length = strlen(charset);
    
    for (int i = 0; i < length; i++) {
        int key = rand() % charset_length;
        password[i] = charset[key];
    }
    password[length] = '\0';
}

// Function to evaluate password strength
const char* evaluate_password_strength(const char *password) {
    int has_upper = 0, has_lower = 0, has_digit = 0, has_special = 0;
    for (int i = 0; password[i] != '\0'; i++) {
        if (isupper(password[i])) has_upper = 1;
        else if (islower(password[i])) has_lower = 1;
        else if (isdigit(password[i])) has_digit = 1;
        else has_special = 1;
    }
    
    int score = has_upper + has_lower + has_digit + has_special;
    
    if (strlen(password) >= 48 && score >= 3) return ":/";
    else if (strlen(password) >= 24 && score >= 3) return "Very strong";
    else if (strlen(password) >= 12 && score >= 3) return "Strong";
    else if (strlen(password) >= 8 && score >= 2) return "Medium";
    else return "Weak. Try to generate another password!";
}

// Callback for toggling password visibility
void on_toggle_visibility(GtkWidget *widget, gpointer entry) {
    gboolean is_visible = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    gtk_entry_set_visibility(GTK_ENTRY(entry), is_visible); //is_visible
}

// Callback for saving the password to a file
void on_save_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *entry = (GtkWidget *)data;
    const char *password = gtk_entry_get_text(GTK_ENTRY(entry));

    GtkWidget *dialog;
    GtkFileChooser *chooser;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;

    dialog = gtk_file_chooser_dialog_new("Save File",
                                         NULL,
                                         action,
                                         "Cancel", GTK_RESPONSE_CANCEL,
                                         "Save", GTK_RESPONSE_ACCEPT,
                                         NULL);
    
    chooser = GTK_FILE_CHOOSER(dialog);
    gtk_file_chooser_set_do_overwrite_confirmation(chooser, TRUE);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename;
        filename = gtk_file_chooser_get_filename(chooser);
        FILE *file = fopen(filename, "w");
        if (file) {
            fprintf(file, "Generated Password: %s\n", password);
            fclose(file);
        }
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

static void on_theme_switch(GtkWidget *button, gpointer user_data) {
    GtkSettings *settings = gtk_settings_get_default();
    static gboolean dark_mode = FALSE;

    if (dark_mode) {
        g_object_set(settings, "gtk-application-prefer-dark-theme", FALSE, NULL);
        dark_mode = FALSE;
    } else {
        g_object_set(settings, "gtk-application-prefer-dark-theme", TRUE, NULL);
        dark_mode = TRUE;
    }
}

// Callback function for the "Generate Password" button
void on_generate_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget **widgets = (GtkWidget **)data;
    GtkWidget *entry = widgets[0];
    GtkWidget *length_entry = widgets[1];
    GtkWidget *strength_label = widgets[2];
    GtkToggleButton *lower_check = GTK_TOGGLE_BUTTON(widgets[3]);
    GtkToggleButton *upper_check = GTK_TOGGLE_BUTTON(widgets[4]);
    GtkToggleButton *digit_check = GTK_TOGGLE_BUTTON(widgets[5]);
    GtkToggleButton *special_check = GTK_TOGGLE_BUTTON(widgets[6]);

    const char *length_str = gtk_entry_get_text(GTK_ENTRY(length_entry));
    int length = atoi(length_str);
    if (length <= 0 || length > 128) length = 12;

    // Get the status of the checkboxes for character sets
    int use_lower = gtk_toggle_button_get_active(lower_check);
    int use_upper = gtk_toggle_button_get_active(upper_check);
    int use_digits = gtk_toggle_button_get_active(digit_check);
    int use_special = gtk_toggle_button_get_active(special_check);

    // If no character set is selected, default to using all sets
    if (!use_lower && !use_upper && !use_digits && !use_special) {
        use_lower = use_upper = use_digits = use_special = 1;
    }

    char *password = (char *)malloc((length + 1) * sizeof(char));
    generate_password(password, length, use_lower, use_upper, use_digits, use_special);
    gtk_entry_set_text(GTK_ENTRY(entry), password);

    // Update the password strength label with prefix "Strength: "
    const char *strength = evaluate_password_strength(password);
    char strength_text[50];
    snprintf(strength_text, sizeof(strength_text), "Strength: %s", strength);
    gtk_label_set_text(GTK_LABEL(strength_label), strength_text);

    free(password);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    srand(time(NULL)); // Seed the random number generator

    GtkWidget *window;
    GtkWidget *button, *save_button;
    GtkWidget *entry;
    GtkWidget *length_entry;
    GtkWidget *strength_label;
    GtkWidget *lower_check, *upper_check, *digit_check, *special_check;
    GtkWidget *visibility_check;
    GtkWidget *box;
    GtkWidget *toggle_button;

    // Create a new window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "GenPassX");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_widget_set_size_request(window, 300, 300);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER); // Center the window

    // Create a vertical box
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), box);

    // Theme button
    button = gtk_button_new_with_label("Switch Theme");
    gtk_widget_set_size_request(button, 15, 15); // Set the size of the button

    // Style the button to remove borders, padding, and add background color
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
       "button { padding: 0px; margin: 0px; border: 0px; background-color: transparent; }", -1, NULL);
    GtkStyleContext *context = gtk_widget_get_style_context(button);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    g_signal_connect(button, "clicked", G_CALLBACK(on_theme_switch), NULL);
    gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 5);

    // Create an entry widget for the generated password
    entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Generated password will appear here");
    gtk_box_pack_start(GTK_BOX(box), entry, FALSE, FALSE, 5);

    // Create an entry widget for the length input
    length_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(length_entry), "Enter length (default 12, max 128)");
    gtk_box_pack_start(GTK_BOX(box), length_entry, FALSE, FALSE, 5);

    // Create a visibility toggle button
    visibility_check = gtk_check_button_new_with_label("Show Password");
    g_signal_connect(visibility_check, "toggled", G_CALLBACK(on_toggle_visibility), entry);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(visibility_check), TRUE);
    gtk_box_pack_start(GTK_BOX(box), visibility_check, FALSE, FALSE, 25);

    // Create checkboxes for character sets
    lower_check = gtk_check_button_new_with_label("Include Lowercase");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lower_check), TRUE);
    gtk_box_pack_start(GTK_BOX(box), lower_check, FALSE, FALSE, 5);

    upper_check = gtk_check_button_new_with_label("Include Uppercase");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(upper_check), TRUE);
    gtk_box_pack_start(GTK_BOX(box), upper_check, FALSE, FALSE, 5);

    digit_check = gtk_check_button_new_with_label("Include Digits");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(digit_check), TRUE);
    gtk_box_pack_start(GTK_BOX(box), digit_check, FALSE, FALSE, 5);

    special_check = gtk_check_button_new_with_label("Include Special Characters");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(special_check), TRUE);
    gtk_box_pack_start(GTK_BOX(box), special_check, FALSE, FALSE, 5);

    toggle_button = gtk_check_button_new_with_label("Auto-clear every 10 seconds");
    gtk_box_pack_start(GTK_BOX(box), toggle_button, FALSE, FALSE, 5);
    GtkWidget *widgets_clear[] = {GTK_WIDGET(toggle_button), GTK_WIDGET(entry)};

    // Create a label to display password strength
    strength_label = gtk_label_new("Strength: ");
    gtk_box_pack_start(GTK_BOX(box), strength_label, FALSE, FALSE, 5);

    // Create a button widget for generating the password
    button = gtk_button_new_with_label("Generate Password");
    GtkWidget *widgets[] = {entry, length_entry, strength_label, lower_check, upper_check, digit_check, special_check, toggle_button};
    g_signal_connect(button, "clicked", G_CALLBACK(on_generate_clicked), widgets);
    gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 5);

    // Create a button widget for saving the password
    save_button = gtk_button_new_with_label("Save Password to File");
    g_signal_connect(save_button, "clicked", G_CALLBACK(on_save_clicked), entry);
    gtk_box_pack_start(GTK_BOX(box), save_button, FALSE, FALSE, 5);

    // Create a button to copy generated password to clickboard
    button = gtk_button_new_with_label("Copy to clickboard");
    g_signal_connect(button, "clicked", G_CALLBACK(copy_to_clipboard), entry);
    gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 5);

    // Set up a timer to clear the entry every 10 seconds (10000 milliseconds)
    g_timeout_add(10000, clear_entry, widgets_clear);

    // Show all widgets
    gtk_widget_show_all(window);

    // Handle the window close event
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Start the GTK main loop
    gtk_main();

    return 0;
}
