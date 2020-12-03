from django.shortcuts import render
from django.http import HttpResponse
import requests
from interface.forms import write_form
from interface.forms import read_form

def index(request):
  context = {}
  w = write_form()
  r = read_form()
  context['w_form'] = w
  context['r_form'] = r
  if request.method == 'GET':
    return render(request, 'interface/interface.html', context)
  device = request.POST['device']
  try:
    value = request.POST['value']
  except:
    value = -1
  r = requests.get("http://74.88.226.33:42069/get", params={'dev':device, 'val':value})
  context['response'] = r.text
  return render(request, 'interface/interface.html', context)