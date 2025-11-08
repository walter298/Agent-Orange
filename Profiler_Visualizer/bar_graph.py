from stats import Stats
import tkinter as tk
from tkinter import ttk

from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure

def make_bar_graph(functions : list[Stats]): 
    sorted_functions = sorted(functions, key=lambda f: f.average(), reverse=True)
    
    fig = Figure(figsize=(5, 4), dpi=100)
    
    ax = fig.add_subplot(111)
    categories = [f.name() for f in sorted_functions]
    values = [f.average() for f in sorted_functions]
    ax.bar(categories, values, color='skyblue')
    ax.set_title("Function Average Times")
    ax.set_xlabel("Function Name")
    ax.set_ylabel("Average (ns)")
    
    return fig

if __name__ == "__main__":
    root = tk.Tk()
    root.title("Simple Bar Graph")

    stats = [
        Stats("func_a", 120, 30.0),
        Stats("func_b", 80, 20.0),
        Stats("func_c", 200, 50.0)
    ]
    
    fig = make_bar_graph(stats)
    
    canvas = FigureCanvasTkAgg(fig, master=root)
    canvas.draw()
    canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)
    
    root.mainloop()