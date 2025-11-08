import tkinter as tk
from tkinter import ttk

from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure

import stats_loader
from bar_graph import make_bar_graph
from pie_chart import make_pie_chart
from stats import Stats

class Assignable:
    def __init__(self):
        self.value = None
    
    def assign(self, val):
        self.value = val

def on_function_select(event, canvas, stats):
    selected_function = event.widget.get()
    function_stats = next((stat for stat in stats if stat.name() == selected_function), None)
    
    if function_stats:
        fig = make_pie_chart(function_stats.child_stats())
        
        if canvas.value:
            canvas.value.get_tk_widget().destroy()
        canvas.assign(FigureCanvasTkAgg(fig, master=event.widget.master))
        canvas.value.draw()
        canvas.value.get_tk_widget().pack(fill=tk.BOTH, expand=True)

def make_function_dropdown(root, stats):
    canvas = Assignable()
    function_names = [stat.name() for stat in stats]
    combo = ttk.Combobox(root, values=function_names, state="readonly")
    combo.bind("<<ComboboxSelected>>", lambda event: on_function_select(event, canvas, stats))
    combo.current(0)
    combo.pack()
    
def on_session_select(event, canvas, stats_map):
    selected_session = event.widget.get()
    stats = stats_map[selected_session]
    
    fig = make_bar_graph(stats)
    
    if canvas.value:
        canvas.value.get_tk_widget().destroy()
    canvas.assign(FigureCanvasTkAgg(fig, master=event.widget.master))
    canvas.value.draw()
    canvas.value.get_tk_widget().pack(fill=tk.BOTH, expand=True)
    
    make_function_dropdown(event.widget.master, stats)

def make_session_dropdown(root, stats_map):
    canvas = Assignable()
    session_names = list(stats_map.keys())
    combo = ttk.Combobox(root, values=session_names, state="readonly")
    combo.bind("<<ComboboxSelected>>", lambda event: on_session_select(event, canvas, stats_map))
    combo.current(0)
    combo.pack()
    
def main():
    root = tk.Tk()

    stats_map = stats_loader.load_stats_map()
    session_names = []
    for session_name in stats_map:
        session_names.append(session_name)
    root.title("Profiling Sessions")

    make_session_dropdown(root, stats_map)
    
    root.mainloop()

if __name__ == "__main__":
    main()