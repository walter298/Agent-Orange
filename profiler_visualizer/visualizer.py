import tkinter as tk
from tkinter import ttk
from tkinter import font

from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2Tk
from matplotlib.figure import Figure

from stats_loader import load_stats_map
from bar_graph import BarGraph
from bar_graph_view import *
from grid_layout import *
from stats import Stats

showingBarGraphs = True

def switch_view(frame, row, session_map, font, bar_graph):
    if showingBarGraphs:
        make_session_bar_graph_dropdown(frame, row, session_map, font, bar_graph)
    else:
        x = 0
        
def make_view_dropdown(frame, row, font):
    options = ['Individual Benchmarks', 'Function Percentages']
    selected_option = tk.StringVar(value=options[0])
    view_dropdown = tk.OptionMenu(frame, selected_option, *options)
    view_dropdown.config(font=font)
    view_dropdown.grid(row=row, column=0, sticky='w', pady=10)
    
def make_dropdown_frame(root, session_map, bar_graph):
    frame = tk.Frame(root)
    
    font = ('Century', 14)
    
    make_view_dropdown(frame, 0, font)
    make_session_bar_graph_dropdown(frame, 1, session_map, font, bar_graph)
    
    return frame
    
def main():
    root = tk.Tk()
    root.title("Agent Orange Profiler")
    root.state('zoomed')
    
    bar_graph = BarGraph(root)
    
    session_map = load_stats_map()
    
    dropdown_frame = make_dropdown_frame(root, session_map, bar_graph)
    dropdown_frame.grid(row=DROPDOWN_ROW, column=0, sticky='w')
    
    root.grid_rowconfigure(BAR_GRAPH_ROW, weight=1)
    root.grid_columnconfigure(0, weight=1) 
    
    root.mainloop()

if __name__ == "__main__":
    main()