from stats import Stats

from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2Tk
from matplotlib.figure import Figure

from grid_layout import *

class BarGraph:
    def __init__(self, root):
        self.fig = Figure(figsize=(5, 4), dpi=100)
        self.ax = self.fig.add_subplot(111)
        self.canvas = FigureCanvasTkAgg(self.fig, master=root)
        self.toolbar = NavigationToolbar2Tk(self.canvas, root)
        self.toolbar.update()
        self.toolbar.grid(row=BAR_GRAPH_ROW+1, column=0, sticky="w")  
        self.canvas.get_tk_widget().grid(row=BAR_GRAPH_ROW, column=0, sticky="nsew")
        
        self.set_axes([])
        
    def set_axes(self, functions: list[Stats]):
        self.canvas.get_tk_widget().grid(row=BAR_GRAPH_ROW, column=0, sticky="nsew") 
        
        sorted_functions = sorted(functions, key=lambda f: f.average(), reverse=True)
        if len(sorted_functions) > 0:
            del sorted_functions[0]
        
        self.ax.clear()
        categories = [f.name() for f in sorted_functions]
        values = [f.average() for f in sorted_functions]
        
        x_pos = range(len(categories))
        self.ax.bar(x_pos, values, color='skyblue')             
        self.ax.set_xticks(x_pos)
        self.ax.set_xticklabels(categories, rotation=45, ha="right")
        
        self.ax.set_title("Function Average Times")
        self.ax.set_xlabel("Function Name")
        self.ax.set_ylabel("Average (ns)")
        
        self.fig.tight_layout()  
        self.canvas.draw()
    
    def clear(self):
        self.ax.clear()          
        self.canvas.draw()