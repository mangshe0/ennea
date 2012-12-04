class 'LivingObject' (Object)

function LivingObject:__init()
  Object.__init(self)
	self:addStat(Stat("Strength"))
	self:addStat(Stat("Physical resistance"))
end

function LivingObject:learn(stat,amount)
	stat.exp += amount
	self:onGainExp(stat, amount)
	local x = stat.max
	while stat.exp > stat.exp_required do
		stat.max = stat.max + 1
		stat.current = stat.current + 1
		stat.exp = stat.exp - stat.exp_required
		stat.exp_required = stat.exp_required * 1.1
		self:onLevelup(stat)
	end
	return stat.max - x;	
end

function LivingObject:onGainExp(stat, amount)
end

function LivingObject:onLevelup(stat)
end

function LivingObject:onDamage(type,amount)
	resistance = self.getStat(type.name() .. " resistance");
	self:learn(resistance, amount);
	true_damage = (amount - resistance.current/10) * (100 - resistance.current * 100 / (resistance.current + 100)) / 100
	Object.onDamage(self,type,true_damage)
end

function LivingObject:onDeath()
    print(self.name .. " is defeated")
end

class 'SimplePoisonBuff' (Buff)

function SimplePoisonBuff:__init(dot)
    Buff.__init(self)
    self.timeLeft = 60
    self.effectCooldown = 10
    self.dot = dot
end

function SimplePoisonBuff:onEffect(carry)
    carry:damage(DamageType.Poison(), self.dot)
end

class 'Poison' (Ability)

function Poison:__init()
    Ability.__init(self)
end

function Poison:onCast(caster, victim)
    victim:applyBuff(SimplePoisonBuff(10))
end

poison = Poison()

warrior = LivingObject()
warrior.name = "Prince Harald IV."
warrior.hp = Stat(99)
warrior.strength = Stat(20)
warrior:setResistance(DamageType.Physical(),Stat(10))

bear = LivingObject()
bear.name = "Grizzly bear"
bear.hp = Stat(50);
bear.strength = Stat(10);
bear:setResistance(DamageType.Physical(), Stat(15))
bear:setResistance(DamageType.Poison(), Stat(5))


