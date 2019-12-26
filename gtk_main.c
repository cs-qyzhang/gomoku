#include <gtk/gtk.h>
#include "FiveChess.h"

#define SEARCH_DEPTH    4

GtkWidget *window;
GtkWidget *buttons[6][6];
gulong buttons_handler_id[6][6];

void quick_message(gchar *message)
{
    GtkWidget *dialog, *label, *content_area;
    GtkDialogFlags flags;

    // Create the widgets
    flags = GTK_DIALOG_DESTROY_WITH_PARENT;
    dialog = gtk_dialog_new_with_buttons("Message",
                                         GTK_WINDOW(window),
                                         flags,
                                         "OK",
                                         GTK_RESPONSE_NONE,
                                         NULL);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    label = gtk_label_new(message);

    // Ensure that the dialog box is destroyed when the user responds
    g_signal_connect(dialog,
                     "response",
                     G_CALLBACK(gtk_main_quit),
                     NULL);

    // Add the label, and show everything we’ve added
    gtk_container_add(GTK_CONTAINER(content_area), label);
    gtk_widget_show_all(dialog);
}

static void toggled(GtkWidget *widget, gpointer data)
{
    // 得到该棋子的坐标
    const char *name = gtk_widget_get_name(widget);
    int min_row = name[7] - '0';
    int min_col = name[9] - '0';

    // 将该棋子设为黑色
    GtkStyleContext *context;
    context = gtk_widget_get_style_context(widget);
    gtk_style_context_add_class(context, "black");
    // 取消该棋子的信号处理
    g_signal_handler_disconnect(widget, buttons_handler_id[min_row][min_col]);

    // AI 下棋
    int row, col;
    int win;
    win = five_chess_play(min_row, min_col, SEARCH_DEPTH, &row, &col);

    // 如果获胜则输出信息
    if (win == MIN_PLAYER) {
        g_print("human win!\n");
        quick_message("Human Win!\n");
    }

    // 将AI下的棋子设为白色并取消该棋子的信号处理
    context = gtk_widget_get_style_context(buttons[row][col]);
    gtk_style_context_add_class(context, "white");
    g_signal_handler_disconnect(buttons[row][col], buttons_handler_id[row][col]);

    // 如果获胜则输出信息
    if (win == MAX_PLAYER) {
        quick_message("AI Win!");
    } else if (win == DRAW) {
        quick_message("Draw!");
    }
}

int main(int argc, char *argv[])
{
    GtkBuilder *builder;
    GObject *button;
    GError *error = NULL;

    gtk_init(&argc, &argv);
    // 初始化棋盘
    init_board(6);

    /* Construct a GtkBuilder instance and load our UI description */
    builder = gtk_builder_new();
    if (gtk_builder_add_from_resource(builder, "/org/gtk/gomoku/FiveChess.ui", &error) == 0) {
        g_printerr("Error loading file: %s\n", error->message);
        g_clear_error(&error);
        return 1;
    }

    /* Connect signal handlers to the constructed widgets. */
    window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // 从.ui文件中获取所有按钮并保存到相应数组中
    char id[] = "button_0_0";
    for (int row = 0; row < 6; ++row) {
        for (int col = 0; col < 6; ++col) {
            id[7] = row + '0';
            id[9] = col + '0';
            button = gtk_builder_get_object(builder, id);
            buttons_handler_id[row][col] = g_signal_connect(button, "toggled", G_CALLBACK(toggled), NULL);
            gtk_widget_set_name(GTK_WIDGET(button), id);
            buttons[row][col] = GTK_WIDGET(button);
        }
    }

    // 设置.css文件
    GtkCssProvider *provider;
    provider = gtk_css_provider_new();
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_css_provider_load_from_resource(provider, "/org/gtk/gomoku/style.css");

    gtk_widget_show_all(GTK_WIDGET(window));
    gtk_main();

    return 0;
}
