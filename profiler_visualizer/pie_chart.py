import tkinter as tk
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure

class Slice:
    def __init__(self, function_name : str, percentage : float):
        self.function_name = function_name
        self.percentage = percentage
        
class PieChart:
    def __init__(self, slices : list[Slice]):
        total_percentage = sum(slice.percentage for slice in slices)
        missing = 100.0 - total_percentage
        slices.append(Slice("other", missing))
        self.slices = slices
        