module.exports = function (ctx, done) {
  var pnumber = ctx.data.number;
  console.log(pnumber);

    ctx.storage.get(function (error, data) {
        if (error) return cb(error);

        ctx.storage.set(pnumber, function (error) {
            if (error) return cb(error);
            
            done(null, pnumber);
        });
    });
};