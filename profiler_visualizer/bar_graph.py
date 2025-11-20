from stats import Stats

from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2Tk
from matplotlib.figure import Figure

from grid_layout import *
from gui import GUI

def show_bar_graph()
    gui.canvas.get_tk_widget().grid(row=BAR_GRAPH_ROW, column=0, sticky="nsew") 
    
    sorted_functions = sorted(functions, key=lambda f: f.average(), reverse=True)
    if len(sorted_functions) > 0:
        del sorted_functions[0]
    
    gui.ax.clear()
    categories = [f.name() for f in sorted_functions]
    values = [f.average() for f in sorted_functions]
    
    x_pos = range(len(categories))
    gui.ax.bar(x_pos, values, color='skyblue')             
    gui.ax.set_xticks(x_pos)
    gui.ax.set_xticklabels(categories, rotation=45, ha="right")
    
    gui.ax.set_title("Function Average Times")
    gui.ax.set_xlabel("Function Name")
    gui.ax.set_ylabel("Average (ns)")
    
    gui.fig.tight_layout()  
    gui.canvas.draw()
    
    def clear(self):
        self.ax.clear()          
        self.canvas.draw()