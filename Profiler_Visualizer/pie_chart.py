import stats

import matplotlib.pyplot as plt

def make_pie_chart(stats_list):
    labels = []
    sizes = []
    for stat in stats_list:
        labels.append(stat.name())
        sizes.append(stat.average())
    plt.pie(sizes, labels=labels, autopct='%1.1f%%')
    plt.axis('equal')
    return plt.gcf()