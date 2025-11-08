class ChildStats:
    def __init__(self, name : str, percentage : float):
        self._name = name
        self._percentage = percentage
        
class Stats:
    def __init__(self, name : str, average : float, child_stats : list[ChildStats]):
        self._name = name
        self._average = average
        self._child_stats = child_stats

    def name(self):
        return self._name

    def average(self):
        return self._average

    def child_stats(self):
        return self._child_stats