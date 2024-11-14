#include <gtk/gtk.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

// version = 0.3.0

// Function to hash the password using SHA256
void hash_password_sha256(const char *password, unsigned char *output_hash) {
    EVP_MD_CTX *mdctx;
    if((mdctx = EVP_MD_CTX_new()) == NULL) {
        g_print("Error creating context for SHA256\n");
        return;
    }

    if(1 != EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL)) {
        g_print("Error initializing SHA256\n");
        EVP_MD_CTX_free(mdctx);
        return;
    }

    if(1 != EVP_DigestUpdate(mdctx, password, strlen(password))) {
        g_print("Error updating SHA256\n");
        EVP_MD_CTX_free(mdctx);
        return;
    }

    if(1 != EVP_DigestFinal_ex(mdctx, output_hash, NULL)) {
        g_print("Error finalizing SHA256\n");
        EVP_MD_CTX_free(mdctx);
        return;
    }

    EVP_MD_CTX_free(mdctx);
}

// Function to hash the password using SHA512
void hash_password_sha512(const char *password, unsigned char *output_hash) {
    EVP_MD_CTX *mdctx;
    if((mdctx = EVP_MD_CTX_new()) == NULL) {
        g_print("Error creating context for SHA512\n");
        return;
    }

    if(1 != EVP_DigestInit_ex(mdctx, EVP_sha512(), NULL)) {
        g_print("Error initializing SHA512\n");
        EVP_MD_CTX_free(mdctx);
        return;
    }

    if(1 != EVP_DigestUpdate(mdctx, password, strlen(password))) {
        g_print("Error updating SHA512\n");
        EVP_MD_CTX_free(mdctx);
        return;
    }

    if(1 != EVP_DigestFinal_ex(mdctx, output_hash, NULL)) {
        g_print("Error finalizing SHA512\n");
        EVP_MD_CTX_free(mdctx);
        return;
    }

    EVP_MD_CTX_free(mdctx);
}

// Function to hash the password using PBKDF2
void hash_password_pbkdf2(const char *password, unsigned char *output_hash, int iterations) {
    unsigned char salt[16];
    if(RAND_bytes(salt, sizeof(salt)) != 1) {
        g_print("Error generating salt\n");
        return;
    }

    if(PKCS5_PBKDF2_HMAC(password, strlen(password), salt, sizeof(salt), iterations, EVP_sha256(), 32, output_hash) != 1) {
        g_print("Error generating PBKDF2 hash\n");
        return;
    }
}

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
void generate_password(char *password, int length, int use_lower, int use_upper, int use_digits, int use_special, int avoid_ambiguous) {
    const char lower[] = "abcdefghijkmnopqrstuvwxyz"; // Excluding 'l'
    const char upper[] = "ABCDEFGHJKLMNPQRSTUVWXYZ"; // Excluding 'I' and 'O'
    const char digits[] = "23456789"; // Excluding '0' and '1'
    const char special[] = "!@#$%^&*";
    char charset[128] = "";

    if (avoid_ambiguous) {
        if (use_lower) strcat(charset, lower);
        if (use_upper) strcat(charset, upper);
        if (use_digits) strcat(charset, digits);
    } else {
        if (use_lower) strcat(charset, "abcdefghijklmnopqrstuvwxyz");
        if (use_upper) strcat(charset, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        if (use_digits) strcat(charset, "0123456789");
    }
    if (use_special) strcat(charset, special);
    
    int charset_length = strlen(charset);
    
    for (int i = 0; i < length; i++) {
        int key = rand() % charset_length;
        password[i] = charset[key];
    }
    password[length] = '\0';
}

// Load css to change color of progressbar
void load_css() {
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "progressbar { background-color: #e0e0e0; }"
        ".very-weak { background-color: red; }"
        ".weak { background-color: orange; }"
        ".medium { background-color: yellow; }"
        ".strong { background-color: lightgreen; }"
        ".very-strong { background-color: green; }",
        -1, NULL);

    GtkStyleContext *context = gtk_style_context_new();
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

const char* evaluate_password_strength(GtkWidget *progress_bar, const char *password) {
    if (progress_bar == NULL || password == NULL) {
        return "Error: Invalid progress bar or password";
    }

    int has_upper = 0, has_lower = 0, has_digit = 0, has_special = 0;
    for (int i = 0; password[i] != '\0'; i++) {
        if (isupper(password[i])) has_upper = 1;
        else if (islower(password[i])) has_lower = 1;
        else if (isdigit(password[i])) has_digit = 1;
        else has_special = 1;
    }

    int score = has_upper + has_lower + has_digit + has_special;
    GtkStyleContext *context = gtk_widget_get_style_context(progress_bar);

    if (strlen(password) >= 48 && score >= 3) {
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), 0.8);
        gtk_style_context_add_class(context, "very-strong");
        return "Very strong";
    } else if (strlen(password) >= 24 && score >= 3) {
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), 0.6);
        gtk_style_context_add_class(context, "strong");
        return "Strong";
    } else if (strlen(password) >= 12 && score >= 3) {
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), 0.4);
        gtk_style_context_add_class(context, "medium");
        return "Medium";
    } else if (strlen(password) >= 8 && score >= 2) {
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), 0.2);
        gtk_style_context_add_class(context, "weak");
        return "Weak";
    } else {
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), 0.1);
        gtk_style_context_add_class(context, "very-weak");
        return "Very Weak!";
    }
}

// Callback for toggling password visibility
void on_toggle_visibility(GtkWidget *widget, gpointer entry) {
    gboolean is_visible = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    gtk_entry_set_visibility(GTK_ENTRY(entry), is_visible);
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
    GtkWidget *length_scale = widgets[1];
    GtkWidget *strength_label = widgets[2];
    GtkToggleButton *lower_check = GTK_TOGGLE_BUTTON(widgets[3]);
    GtkToggleButton *upper_check = GTK_TOGGLE_BUTTON(widgets[4]);
    GtkToggleButton *digit_check = GTK_TOGGLE_BUTTON(widgets[5]);
    GtkToggleButton *special_check = GTK_TOGGLE_BUTTON(widgets[6]);
    GtkToggleButton *ambiguous_check = GTK_TOGGLE_BUTTON(widgets[7]);
    GtkComboBoxText *algorithm_combo = GTK_COMBO_BOX_TEXT(widgets[8]);
    GtkWidget *progress_bar = widgets[9];

    // Get the length value from the slider
    int length = (int)gtk_range_get_value(GTK_RANGE(length_scale));

    // Get the status of the checkboxes for character sets
    int use_lower = gtk_toggle_button_get_active(lower_check);
    int use_upper = gtk_toggle_button_get_active(upper_check);
    int use_digits = gtk_toggle_button_get_active(digit_check);
    int use_special = gtk_toggle_button_get_active(special_check);
    int avoid_ambiguous = gtk_toggle_button_get_active(ambiguous_check);

    // If no character set is selected, default to using all sets
    if (!use_lower && !use_upper && !use_digits && !use_special) {
        use_lower = use_upper = use_digits = use_special = 1;
    }

    char *password = (char *)malloc((length + 1) * sizeof(char));
    generate_password(password, length, use_lower, use_upper, use_digits, use_special, avoid_ambiguous);
    gtk_entry_set_text(GTK_ENTRY(entry), password);

    // Update the password strength label with prefix "Strength: "
    const char *strength = evaluate_password_strength(progress_bar, password);
    char strength_text[50];
    snprintf(strength_text, sizeof(strength_text), "Strength: %s", strength);
    gtk_label_set_text(GTK_LABEL(strength_label), strength_text);

    // Get the selected algorithm from the dropdown
    const gchar *selected_algorithm = gtk_combo_box_text_get_active_text(algorithm_combo);
    if (selected_algorithm) {
        unsigned char output_hash[64]; // Maximum size needed for SHA512
        if (strcmp(selected_algorithm, "SHA256") == 0) {
            hash_password_sha256(password, output_hash);
            g_print("SHA256 hash generated\n");
        } else if (strcmp(selected_algorithm, "SHA512") == 0) {
            hash_password_sha512(password, output_hash);
            g_print("SHA512 hash generated\n");
        } else if (strcmp(selected_algorithm, "PBKDF2") == 0) {
            hash_password_pbkdf2(password, output_hash, 10000);
            g_print("PBKDF2 hash generated\n");
        }
    }

    free(password);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    load_css(); // For progressbar colors
		//
    srand(time(NULL)); // Seed the random number generator

    GtkWidget *window;
    GtkWidget *button, *save_button;
    GtkWidget *entry;
    GtkWidget *length_scale;
    GtkWidget *strength_label;
    GtkWidget *lower_check, *upper_check, *digit_check, *special_check, *ambiguous_check;
    GtkWidget *visibility_check;
    GtkWidget *box;
    GtkWidget *toggle_button;
    GtkWidget *algorithm_combo;
    GtkWidget *progress_bar;

    // Create a new window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "GenPassX");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_widget_set_size_request(window, 300, 400);
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

    // Create a scale (slider) for the password length
    length_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 1, 128, 1);
    gtk_range_set_value(GTK_RANGE(length_scale), 12); // Set default value to 12
    gtk_scale_set_digits(GTK_SCALE(length_scale), 0); // Set to integer values
    gtk_scale_set_value_pos(GTK_SCALE(length_scale), GTK_POS_RIGHT);
    gtk_box_pack_start(GTK_BOX(box), length_scale, FALSE, FALSE, 5);

    // Create a visibility toggle button
    visibility_check = gtk_check_button_new_with_label("Show Password");
    g_signal_connect(visibility_check, "toggled", G_CALLBACK(on_toggle_visibility), entry);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(visibility_check), TRUE);
    gtk_box_pack_start(GTK_BOX(box), visibility_check, FALSE, FALSE, 5);

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

    ambiguous_check = gtk_check_button_new_with_label("Avoid Ambiguous Characters");
    gtk_box_pack_start(GTK_BOX(box), ambiguous_check, FALSE, FALSE, 5);

    toggle_button = gtk_check_button_new_with_label("Auto-clear every 10 seconds");
    gtk_box_pack_start(GTK_BOX(box), toggle_button, FALSE, FALSE, 5);
    GtkWidget *widgets_clear[] = {GTK_WIDGET(toggle_button), GTK_WIDGET(entry)};

    // Create a label to display password strength
    strength_label = gtk_label_new("Strength: ");
    gtk_box_pack_start(GTK_BOX(box), strength_label, FALSE, FALSE, 5);

    // Progress bar for strength
    progress_bar = gtk_progress_bar_new();
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), 0.0);
    gtk_box_pack_start(GTK_BOX(box), progress_bar, FALSE, FALSE, 5);
    //gtk_container_add(GTK_CONTAINER(window), progress_bar); RM RM RM

    // Create a dropdown for password strengthening algorithms
    algorithm_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(algorithm_combo), NULL, "None");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(algorithm_combo), NULL, "PBKDF2");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(algorithm_combo), NULL, "SHA256");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(algorithm_combo), NULL, "SHA512");
    gtk_combo_box_set_active(GTK_COMBO_BOX(algorithm_combo), 0); // Set default to "None"
    gtk_box_pack_start(GTK_BOX(box), algorithm_combo, FALSE, FALSE, 5);

    // Create a button widget for generating the password
    button = gtk_button_new_with_label("Generate Password");
    GtkWidget *widgets[] = {entry, length_scale, strength_label, lower_check, upper_check, digit_check, special_check, ambiguous_check, algorithm_combo, progress_bar};
    g_signal_connect(button, "clicked", G_CALLBACK(on_generate_clicked), widgets);
    gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 5);

    // Create a button widget for saving the password
    save_button = gtk_button_new_with_label("Save Password to File");
    g_signal_connect(save_button, "clicked", G_CALLBACK(on_save_clicked), entry);
    gtk_box_pack_start(GTK_BOX(box), save_button, FALSE, FALSE, 5);

    // Create a button to copy generated password to clipboard
    button = gtk_button_new_with_label("Copy to clipboard");
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
