from django.db import models

class MotorControl(models.Model):
  motor_id = models.IntegerField()
  motor_pos = models.IntegerField()
