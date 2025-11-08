import tkinter as tk

def make_session_bar_graph_dropdown(frame, row, session_map, font, bar_graph):
    options = [session_name for session_name in session_map.keys()]
    selected_option  = tk.StringVar(value=options[-1])
    session_dropdown = tk.OptionMenu(frame, selected_option, *options)
    selected_option.trace_add('write', lambda *args: bar_graph.set_axes(session_map[selected_option.get()]))
    session_dropdown.config(font=font)
    session_dropdown.grid(row=row, column=0, sticky='w')

