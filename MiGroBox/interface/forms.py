from django import forms

write_choices = [
  ('fan_1', 'Fan 1'),
  ('fan_2', 'Fan 2'),
  ('lights', 'Lights'),
  ('motor', 'Motor'),
  ('pump', 'Water Pump'),
]

read_choices = [
  ('fan_1', 'Fan 1'),
  ('fan_2', 'Fan 2'),
  ('humidity', 'Humidity'),
  ('lights', 'Lights'),
  ('', 'Grow Tray Load Cell 1'),
  ('', 'Grow Tray Load Cell 2'),
  ('', 'Grow Tray Load Cell 3'),
  ('', 'Grow Tray Load Cell 4'),
  ('motor', 'Motor'),
  ('temp', 'Temperature'),
  ('pump', 'Water Pump'),
]

class write_form(forms.Form):
  device = forms.ChoiceField(choices = write_choices)
  value = forms.IntegerField()

class read_form(forms.Form):
  device = forms.ChoiceField(choices = read_choices)