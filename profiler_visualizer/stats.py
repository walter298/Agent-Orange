from pie_chart import PieChart

class Stats:
    def __init__(self, name : str, average : float, pie_chart : PieChart):
        self._name = name
        self._average = average
        self._pie_chart = pie_chart

    def name(self):
        return self._name

    def average(self):
        return self._average

    def pie_chart(self):
        return self._pie_chart
