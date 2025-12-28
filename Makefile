all:
	gcc main_ui.c queue_manager.c reporting.c -o cinema_app -Wall

clean:
	rm -f cinema_app cinema_app.exe