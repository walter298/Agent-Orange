import tkinter as tk
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2Tk
from matplotlib.figure import Figure

from stats_loader import load_stats_map

VIEW_ROW = 0
SESSION_ROW = 1

BENCHMARK_VIEW = "Individual Benchmarks"
PERCENTAGE_VIEW = "Function Percentages"

class GUI:
    def __init__(self):
        self.root = tk.Tk()
        self.root.title("Agent Orange Profiler")
        self.root.state('zoomed')

        self.font = ('Century', 14)
        self.toolbar = None

        self.root.grid_rowconfigure(1, weight=1) 
        self.root.grid_columnconfigure(0, weight=1)

        self.dropdown_frame = tk.Frame(self.root)
        self.dropdown_frame.grid(row=0, column=0, sticky='nw', padx=10, pady=10)

        self.graph_frame = tk.Frame(self.root)
        self.graph_frame.grid(row=1, column=0, sticky="nsew") 
        
        self.fig = Figure(figsize=(5, 4), dpi=100)
        
        self.canvas = FigureCanvasTkAgg(self.fig, master=self.graph_frame)
        self.canvas.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=True)

        self.map = load_stats_map() 
        self.selected_session = list(self.map.keys())[-1]
        self.view_option = BENCHMARK_VIEW

        self.make_view_dropdown(self.dropdown_frame)
        self.make_session_dropdown(self.dropdown_frame)

        self.update_view(self.selected_session)
        
    def make_view_dropdown(self, frame):
        options = [BENCHMARK_VIEW, PERCENTAGE_VIEW]
        self.view_var = tk.StringVar(value=options[0])
        self.view_var.trace_add("write", lambda *args: self.swap_view(self.view_var.get()))
        
        view_dropdown = tk.OptionMenu(frame, self.view_var, *options)
        view_dropdown.config(font=self.font)
        view_dropdown.grid(row=VIEW_ROW, column=0, sticky='w', pady=10)

    def make_session_dropdown(self, frame):
        options = [session_name for session_name in self.map.keys()]
        self.session_var = tk.StringVar(value=options[-1])
        self.session_var.trace_add('write', lambda *args: self.update_view(self.session_var.get()))
        
        session_dropdown = tk.OptionMenu(frame, self.session_var, *options)
        session_dropdown.config(font=self.font)
        session_dropdown.grid(row=SESSION_ROW, column=0, sticky='w')

    def clear(self):
        self.fig.clf() 

    def show_bar_graph(self, functions: list):
        self.clear()
        
        if self.toolbar:
            self.toolbar.destroy()
        
        self.toolbar = NavigationToolbar2Tk(self.canvas, self.graph_frame)
        self.toolbar.update()

        self.ax = self.fig.add_subplot(111)

        sorted_functions = sorted(functions, key=lambda f: f.average(), reverse=True)
        if len(sorted_functions) > 0:
            del sorted_functions[0] 

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

    def show_pie_charts(self, functions: list):
        self.clear()
        
        if self.toolbar:
            self.toolbar.update()

        graphable_functions = [f for f in functions if f.pie_chart()]
        
        if len(graphable_functions) == 0:
            self.canvas.draw()
            return
        
        cols = min(len(graphable_functions), 3)
        rows = (len(graphable_functions) + cols - 1) // cols

        axes = self.fig.subplots(rows, cols, squeeze=False)

        i = 0
        for r in range(rows):
            for c in range(cols):
                if i >= len(graphable_functions):
                    break
                
                stat = graphable_functions[i]
                chart_data = stat.pie_chart() 
            
                labels = [s.function_name for s in chart_data.slices]
                sizes = [s.percentage for s in chart_data.slices]
                
                axes[r, c].pie(sizes, labels=labels, autopct='%3.1f%%', startangle=90)
                axes[r, c].set_title(stat.name())
                
                i += 1

        self.fig.tight_layout()
        self.canvas.draw()
            
    def update_view(self, selected_session):
        self.selected_session = selected_session
        if self.view_option == BENCHMARK_VIEW:
            self.show_bar_graph(self.map[self.selected_session])
        else:
            self.show_pie_charts(self.map[self.selected_session])

    def swap_view(self, view_option):
        self.view_option = view_option
        self.update_view(self.selected_session)

    def show(self):
        self.root.mainloop()